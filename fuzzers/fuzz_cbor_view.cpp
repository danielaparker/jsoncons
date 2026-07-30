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
        auto reparsed = parse_item(bytes);
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

        // read_item agrees with scan on outcome and consumed length.
        wire_cursor items(input);
        scan_context context;
        auto read = items.read_item(context);
        auto scanned = scan(input, context);
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

        // skip_item agrees with read_item on outcome, consumed length, and
        // failure; on success its span is the item's encoded bytes unparsed.
        wire_cursor skips(input);
        auto skipped = skips.skip_item(context);
        require(skipped.has_value() == read.has_value());
        require(skips.position() == items.position());
        if (skipped.has_value())
        {
            require(skipped.value().data() == read.value().encoded_bytes().data());
            require(skipped.value().size() == read.value().encoded_bytes().size());
        }
        else
        {
            require(skipped.error().code == read.error().code);
            require(skipped.error().offset == read.error().offset);
        }

        // skip consumes exactly the requested content when it is available.
        wire_cursor content(input);
        const std::size_t half = input.size() / 2;
        require(content.skip(half));
        require(content.position() == half);
        require(!content.skip(input.size() - half + 1));
        require(content.position() == half);
        require(content.skip(input.size() - half));
        require(content.remaining().empty());
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
        auto sa = scan(a, ca);
        scan_context cb;
        auto sb = scan(b, cb);
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

    void exercise_walker(byte_span input, int depth)
    {
        auto result = get_walker(input, depth);
        scan_context context(depth);
        auto scanned = scan(input, context);
        if (!result.has_value())
        {
            require(!scanned.has_value());
            return;
        }

        auto raw = std::move(result.value());
        auto finished = raw.finish_item();
        require(finished.has_value() == scanned.has_value());
        if (!finished.has_value())
        {
            require(finished.error().code == scanned.error().code);
            require(finished.error().offset == scanned.error().offset);
            return;
        }
        require(finished.value().encoded_bytes().data() == input.data());
        require(finished.value().encoded_bytes().size() == scanned.value().first.encoded_bytes().size());

        const byte_span root_bytes = finished.value().encoded_bytes();
        auto checked = validate(root_bytes, depth);
        require(checked.has_value());
        require(checked.value().encoded_bytes().size() == root_bytes.size());

        raw.rewind();
        auto walker = std::move(raw);
        require(walker.role() == position_role::root);
        require(walker.depth() == 0);

        for (int steps = 0; steps < 256; ++steps)
        {
            uint64_t u = 0;
            int64_t i = 0;
            bool b = false;
            double d = 0;
            jsoncons::string_view text;
            byte_span bytes;
            (void)walker.uint64_value(u);
            (void)walker.int64_value(i);
            (void)walker.bool_value(b);
            (void)walker.double_value(d);
            (void)walker.text(text);
            (void)walker.bytes(bytes);

            auto entered = walker.enter();
            require(entered.has_value());
            if (entered.value())
            {
                require(walker.depth() > 0);
                continue;
            }
            auto advanced = walker.next();
            require(advanced.has_value());
            if (advanced.value())
            {
                continue;
            }
            auto left = walker.leave();
            require(left.has_value());
            if (!left.value())
            {
                break;
            }
        }

        walker.rewind();
        require(walker.depth() == 0);
        require(walker.role() == position_role::root);
        auto root = walker.finish_item();
        require(root.has_value());
        require(root.value().encoded_bytes().size() == root_bytes.size());
    }

    void exercise_raw_walker(byte_span input, int depth)
    {
        auto made = get_walker(input, depth);
        if (!made.has_value())
        {
            return;
        }
        auto walker = std::move(made.value());
        for (std::size_t step = 0; step < 64; ++step)
        {
            const item_kind kind = walker.kind();
            const uint64_t argument = walker.argument();
            const position_role role = walker.role();
            const std::size_t walker_depth = walker.depth();
            const bool extent_known = walker.extent_known();
            const uint8_t operation = input.empty()
                ? static_cast<uint8_t>(step)
                : input[(step * 17) % input.size()];

            bool failed = false;
            switch (operation % 5)
            {
                case 0:
                {
                    auto result = walker.enter();
                    failed = !result.has_value();
                    break;
                }
                case 1:
                {
                    auto result = walker.next();
                    failed = !result.has_value();
                    break;
                }
                case 2:
                {
                    auto result = walker.leave();
                    failed = !result.has_value();
                    break;
                }
                case 3:
                {
                    auto result = walker.finish_item();
                    failed = !result.has_value();
                    break;
                }
                default:
                    walker.rewind();
                    break;
            }

            if (failed)
            {
                require(walker.kind() == kind);
                require(walker.argument() == argument);
                require(walker.role() == role);
                require(walker.depth() == walker_depth);
                require(walker.extent_known() == extent_known);
                break;
            }
        }
    }

    // Children agree with walker movement over the same container.
    void exercise_children(byte_span input, scan_context& context)
    {
        auto scanned = scan(input, context);
        if (!scanned.has_value())
        {
            return;
        }
        const item root = scanned.value().first;

        auto result = get_walker(root.encoded_bytes());
        require(result.has_value());
        auto walker = std::move(result.value());
        require(walker.finish_item().has_value());
        walker.rewind();

        auto it = root.children(context).begin();
        auto entered = walker.enter();
        require(entered.has_value());
        if (!entered.value())
        {
            require(root.children(context).empty());
            require(it == item::child_iterator());
            return;
        }
        require(!root.children(context).empty());

        std::size_t count = 0;
        for (;;)
        {
            require(it != item::child_iterator());
            const item child = *it;
            require(child.kind() == walker.kind());
            require(child.argument() == walker.argument());
            require(child.indefinite() == walker.indefinite());
            auto finished = walker.finish_item();
            require(finished.has_value());
            require(finished.value().encoded_bytes().data() == child.encoded_bytes().data());
            require(finished.value().encoded_bytes().size() == child.encoded_bytes().size());
            ++it;
            if (++count > 4096)
            {
                return;
            }
            auto next = walker.next();
            require(next.has_value());
            if (!next.value())
            {
                break;
            }
        }
        require(it == item::child_iterator());
    }


    void exercise_input(byte_span input, scan_context& context)
    {
        auto scanned = scan(input, context);
        auto exact = parse_item(input, context);

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
            auto second = scan(remainder, context);
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
        exercise_raw_walker(input, depth);
        exercise_walker(input, depth);
        exercise_children(input, context);
        exercise_input(byte_span(base, mid), context);
        exercise_walker(byte_span(base, mid), depth);
        exercise_input(byte_span(base + mid, size - mid), context);
        exercise_walker(byte_span(base + mid, size - mid), depth);
        exercise_input(byte_span(base + p0, size - p0), context);
        exercise_walker(byte_span(base + p0, size - p0), depth);
    }

    return 0;
}
