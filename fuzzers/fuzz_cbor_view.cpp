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

        // Containers: children are checked items inside the parent.
        if (depth_budget > 0)
        {
            const uint8_t* prev_end = nullptr;
            for (item element : scanned.elements())
            {
                require(contains(bytes, element.encoded_bytes()));
                if (prev_end != nullptr)
                {
                    require(element.encoded_bytes().data() >= prev_end);
                }
                prev_end = element.encoded_bytes().data() + element.encoded_bytes().size();
                exercise_item(element, depth_budget - 1);
            }
            for (map_entry entry : scanned.entries())
            {
                require(contains(bytes, entry.key.encoded_bytes()));
                require(contains(bytes, entry.value.encoded_bytes()));
                require(entry.value.encoded_bytes().data() ==
                    entry.key.encoded_bytes().data() + entry.key.encoded_bytes().size());
                exercise_item(entry.key, depth_budget - 1);
                exercise_item(entry.value, depth_budget - 1);
            }
        }

        (void)map_keys_sorted(scanned);
        (void)map_keys_sorted(scanned, length_first_compare());
    }

    void exercise_lowlevel(byte_span input)
    {
        const uint8_t* const end = input.data() + input.size();

        // read_head walks heads, always advancing; ok <=> !ec.
        const uint8_t* p = input.data();
        for (int steps = 0; steps < 64 && p < end; ++steps)
        {
            const uint8_t* before = p;
            item_head head;
            std::error_code ec;
            const bool ok = read_head(p, end, head, ec);
            require(ok != static_cast<bool>(ec));
            require(p >= before && p <= end);
            if (!ok)
            {
                break;
            }
            require(p > before);
        }

        // skip_item agrees with scan_prefix on outcome and consumed length.
        const uint8_t* sp = input.data();
        std::error_code ec;
        const bool ok = skip_item(sp, end, ec);
        require(ok != static_cast<bool>(ec));
        scan_context context;
        auto scanned = scan_prefix(input, context);
        require(ok == scanned.has_value());
        if (ok)
        {
            require(sp == input.data() + scanned.value().first.encoded_bytes().size());
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
            require(sorted.value() == map_keys_sorted(sa.value().first));
        }
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
        exercise_input(byte_span(base, mid), context);
        exercise_input(byte_span(base + mid, size - mid), context);
        exercise_input(byte_span(base + p0, size - p0), context);
    }

    return 0;
}
