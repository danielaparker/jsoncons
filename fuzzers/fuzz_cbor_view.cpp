#include <jsoncons_ext/cbor/cbor_view.hpp>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <system_error>
#include <vector>

using namespace jsoncons::cbor::view;

namespace {

    using byte_span = jsoncons::span<const uint8_t>;

    void require(bool condition)
    {
        if (!condition)
        {
            std::abort();
        }
    }

    bool contains(byte_span outer, byte_span inner) noexcept
    {
        return inner.data() >= outer.data() &&
               inner.data() + inner.size() >= inner.data() &&
               inner.data() + inner.size() <= outer.data() + outer.size();
    }

    int sign(int value) noexcept
    {
        return (value > 0) - (value < 0);
    }

    void exercise_item(const item& scanned, int depth_budget)
    {
        const byte_span bytes = scanned.encoded_bytes();
        require(bytes.size() > 0);

        // Self-similarity: an item's encoding is exactly one item.
        auto reparsed = parse_exact(bytes);
        require(reparsed.has_value());
        require(reparsed.value().encoded_bytes().data() == bytes.data());
        require(reparsed.value().encoded_bytes().size() == bytes.size());
        require(reparsed.value().kind() == scanned.kind());
        require(reparsed.value().argument() == scanned.argument());
        require(reparsed.value().indefinite() == scanned.indefinite());

        std::size_t tag_count = 0;
        for (uint64_t tag : scanned.tags())
        {
            (void)tag;
            if (++tag_count > 4096)
            {
                break;
            }
        }

        // Typed accessors are total over checked items.
        uint64_t u = 0;
        int64_t i = 0;
        bool b = false;
        double d = 0;
        const bool u_ok = scanned.uint64_value(u);
        const bool i_ok = scanned.int64_value(i);
        if (u_ok && u <= static_cast<uint64_t>((std::numeric_limits<int64_t>::max)()))
        {
            require(i_ok && i == static_cast<int64_t>(u));
        }
        (void)scanned.bool_value(b);
        (void)scanned.double_value(d);

        jsoncons::string_view text_view;
        std::string text_copy;
        const bool view_ok = scanned.text(text_view);
        const bool copy_ok = scanned.text(text_copy);
        if (view_ok)
        {
            require(copy_ok && text_copy.size() == text_view.size());
            require(text_view.empty() ||
                std::memcmp(text_copy.data(), text_view.data(), text_view.size()) == 0);
        }
        require(copy_ok == (scanned.kind() == item_kind::text_string));

        byte_span bytes_view;
        std::vector<uint8_t> bytes_copy;
        const bool bytes_view_ok = scanned.bytes(bytes_view);
        const bool bytes_copy_ok = scanned.bytes(bytes_copy);
        if (bytes_view_ok)
        {
            require(bytes_copy_ok && bytes_copy.size() == bytes_view.size());
            require(bytes_view.empty() ||
                std::memcmp(bytes_copy.data(), bytes_view.data(), bytes_view.size()) == 0);
        }

        // Chunks partition the copied content.
        std::size_t chunk_total = 0;
        for (byte_span chunk : scanned.chunks())
        {
            require(chunk.size() == 0 || contains(bytes, chunk));
            chunk_total += chunk.size();
        }
        if (copy_ok)
        {
            require(chunk_total == text_copy.size());
        }
        if (bytes_copy_ok)
        {
            require(chunk_total == bytes_copy.size());
        }

        if (scanned.kind() == item_kind::text_string)
        {
            (void)validate_text(scanned);
        }
        else
        {
            require(!validate_text(scanned));
        }

        (void)depth_budget;
        (void)map_keys_sorted(bytes);
        (void)map_keys_sorted(bytes, length_first_compare());
    }

    void exercise_lowlevel(byte_span input)
    {
        // read_head walks heads by offset, always advancing when bytes are
        // consumed; success and error are mutually exclusive.
        wire_cursor heads(input);
        for (int steps = 0; steps < 64 && !heads.remaining().empty(); ++steps)
        {
            const std::size_t before = heads.position();
            auto result = heads.read_head();
            require(heads.position() >= before && heads.position() <= input.size());
            if (!result.has_value())
            {
                require(result.error().offset == heads.position());
                break;
            }
            require(heads.position() > before);
        }

        // read_item agrees with scan_prefix on outcome and consumed length.
        wire_cursor items(input);
        scan_context context;
        auto read = items.read_item(context);
        auto scanned = scan_prefix(input, context);
        require(read.has_value() == scanned.has_value());
        if (read.has_value())
        {
            require(items.position() == scanned.value().first.encoded_bytes().size());
            require(read.value().encoded_bytes().data() == input.data());
        }
        else
        {
            require(items.position() == read.error().offset);
        }
    }

    void exercise_span_orders(byte_span a, byte_span b)
    {
        auto ab = compare(a, b);
        auto ba = compare(b, a);
        if (ab.has_value() && ba.has_value())
        {
            require(sign(ab.value()) == -sign(ba.value()));
        }

        scan_context ca;
        auto sa = scan_prefix(a, ca);
        scan_context cb;
        auto sb = scan_prefix(b, cb);
        if (sa.has_value() && sb.has_value())
        {
            require(ab.has_value());
            require(sign(ab.value()) == sign(bytewise_compare()(sa.value().first, sb.value().first)));
        }
        else
        {
            require(!ab.has_value());
        }

        auto sorted = map_keys_sorted(a);
        require(sorted.has_value() == sa.has_value());
        if (sorted.has_value())
        {
            auto checked = map_keys_sorted(sa.value().first.encoded_bytes());
            require(checked.has_value() && sorted.value() == checked.value());
        }
    }

    void exercise_navigator(byte_span input, int depth)
    {
        auto navigated = navigate_prefix(input, depth);
        scan_context context(depth);
        auto scanned = scan_prefix(input, context);
        require(navigated.has_value() == scanned.has_value());
        if (!navigated.has_value())
        {
            return;
        }

        navigator nav = std::move(navigated.value().first);
        require(navigated.value().remainder.size() == scanned.value().remainder.size());
        require(nav.role() == position_role::root);
        require(nav.depth() == 0);
        require(nav.extent_known());

        const item root = nav.finish_item();
        require(root.encoded_bytes().data() == input.data());
        require(root.encoded_bytes().size() == scanned.value().first.encoded_bytes().size());

        for (int steps = 0; steps < 256; ++steps)
        {
            uint64_t u = 0;
            int64_t i = 0;
            bool b = false;
            double d = 0;
            jsoncons::string_view text;
            byte_span bytes;
            (void)nav.uint64_value(u);
            (void)nav.int64_value(i);
            (void)nav.bool_value(b);
            (void)nav.double_value(d);
            (void)nav.text(text);
            (void)nav.bytes(bytes);

            if (nav.enter())
            {
                require(nav.depth() > 0);
                continue;
            }
            if (nav.next())
            {
                continue;
            }
            if (!nav.leave())
            {
                break;
            }
        }

        nav.rewind();
        require(nav.depth() == 0);
        require(nav.role() == position_role::root);
        require(nav.finish_item().encoded_bytes().size() == root.encoded_bytes().size());

        auto reset = nav.reset_prefix(input, depth);
        require(reset.has_value());
        require(reset.value().size() == navigated.value().remainder.size());
    }


    void exercise_input(byte_span input, scan_context& context)
    {
        auto scanned = scan_prefix(input, context);
        auto exact = parse_exact(input, context);

        if (scanned.has_value())
        {
            const item& first = scanned.value().first;
            const byte_span remainder = scanned.value().remainder;
            require(first.encoded_bytes().data() == input.data());
            require(first.encoded_bytes().size() + remainder.size() == input.size());
            require(remainder.data() == input.data() + first.encoded_bytes().size());

            require(exact.has_value() == remainder.empty());
            if (!exact.has_value())
            {
                require(exact.error().code == jsoncons::cbor::cbor_errc::trailing_data);
                require(exact.error().offset == first.encoded_bytes().size());
            }

            exercise_item(first, 24);

            // Comparison functors agree with their sign-flipped duals.
            auto second = scan_prefix(remainder, context);
            if (second.has_value())
            {
                const int ab = bytewise_compare()(first, second.value().first);
                const int ba = bytewise_compare()(second.value().first, first);
                require(sign(ab) == -sign(ba));
                const int lab = length_first_compare()(first, second.value().first);
                const int lba = length_first_compare()(second.value().first, first);
                require(sign(lab) == -sign(lba));
                require(bytewise_less()(first, second.value().first) == (ab < 0));
                require(length_first_less()(first, second.value().first) == (lab < 0));
            }
        }
        else
        {
            require(!exact.has_value());
            require(scanned.error().offset <= input.size());
            require(scanned.error().code != jsoncons::cbor::cbor_errc::success);
        }
    }

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, std::size_t size)
{
    static const uint8_t empty_input = 0;
    const uint8_t* base = size == 0 ? &empty_input : data;
    byte_span input(base, size);
    const std::size_t p0 = size == 0 ? 0 : base[0] % (size + 1);
    const std::size_t mid = size / 2;

    const int fuzz_depth = size <= 2 ? default_max_nesting_depth : static_cast<int>(base[2] & 0x0f);
    const int depths[] = {0, 1, fuzz_depth, default_max_nesting_depth};

    exercise_lowlevel(input);
    exercise_span_orders(byte_span(base, mid), byte_span(base + mid, size - mid));
    exercise_span_orders(input, byte_span(base + p0, size - p0));

    for (int depth : depths)
    {
        scan_context context(depth);
        exercise_input(input, context);
        exercise_navigator(input, depth);
        exercise_input(byte_span(base, mid), context);
        exercise_navigator(byte_span(base, mid), depth);
        exercise_input(byte_span(base + mid, size - mid), context);
        exercise_navigator(byte_span(base + mid, size - mid), depth);
        exercise_input(byte_span(base + p0, size - p0), context);
        exercise_navigator(byte_span(base + p0, size - p0), depth);
    }

    return 0;
}
