#include <jsoncons_ext/cbor/cbor_view.hpp>

#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
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

    bool same(byte_span a, byte_span b) noexcept
    {
        return a.data() == b.data() && a.size() == b.size();
    }

    // Offset of the first byte after an item's leading tags and head,
    // decoded independently of the library.
    std::size_t content_offset(byte_span bytes)
    {
        std::size_t pos = 0;
        for (;;)
        {
            require(pos < bytes.size());
            const uint8_t initial = bytes[pos];
            const uint8_t info = initial & 0x1f;
            pos += 1 + (info < 24 || info == 31 ? 0 : (std::size_t(1) << (info - 24)));
            if ((initial >> 5) != 6)
            {
                return pos;
            }
        }
    }

    void exercise_item(const item& scanned, int budget);

    // Children agree with successive scans of the content region, tile it
    // exactly, and are each exactly one item.
    void exercise_children(const item& parent, int budget)
    {
        const byte_span bytes = parent.encoded_bytes();
        const std::size_t offset = content_offset(bytes);
        byte_span rest(bytes.data() + offset, bytes.size() - offset);

        const bool definite_container =
            (parent.kind() == item_kind::array || parent.kind() == item_kind::map) && !parent.indefinite();
        const bool has_children = definite_container || parent.indefinite();

        std::size_t count = 0;
        for (const item child : parent.children())
        {
            require(has_children);
            if (parent.indefinite())
            {
                require(!rest.empty() && rest[0] != 0xff);
            }
            auto oracle = scan(rest);
            require(oracle.has_value());
            require(same(child.encoded_bytes(), oracle.value().first.encoded_bytes()));
            require(child.kind() == oracle.value().first.kind());
            require(child.argument() == oracle.value().first.argument());
            require(child.indefinite() == oracle.value().first.indefinite());
            if (parent.kind() == item_kind::byte_string || parent.kind() == item_kind::text_string)
            {
                require(child.kind() == parent.kind() && !child.indefinite() && child.tags().empty());
            }
            rest = oracle.value().remainder;
            ++count;
            if (budget > 0)
            {
                exercise_item(child, budget - 1);
            }
        }

        if (!has_children)
        {
            require(count == 0 && parent.children().empty());
            return;
        }
        require(parent.children().empty() == (count == 0));
        if (parent.indefinite())
        {
            require(rest.size() == 1 && rest[0] == 0xff);
        }
        else
        {
            require(rest.empty());
            require(count == (parent.kind() == item_kind::map ? 2 : 1) * parent.argument());
        }
    }

    void exercise_item(const item& scanned, int budget)
    {
        const byte_span bytes = scanned.encoded_bytes();
        require(bytes.size() > 0);

        // Self-similarity: an item's encoding is exactly one item.
        auto reparsed = parse_item(bytes);
        require(reparsed.has_value());
        require(same(reparsed.value().encoded_bytes(), bytes));
        require(reparsed.value().kind() == scanned.kind());
        require(reparsed.value().argument() == scanned.argument());
        require(reparsed.value().indefinite() == scanned.indefinite());

        // Tags are present exactly when the encoding starts with one.
        std::size_t tag_count = 0;
        for (uint64_t tag : scanned.tags())
        {
            (void)tag;
            ++tag_count;
        }
        require(scanned.tags().empty() == (tag_count == 0));
        require((bytes[0] >> 5) == 6 ? tag_count > 0 : tag_count == 0);

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

        // Copies agree with views, and assemble chunked strings from children.
        jsoncons::string_view text_view;
        std::string text_copy;
        const bool view_ok = scanned.text(text_view);
        const bool copy_ok = scanned.text(text_copy);
        require(copy_ok == (scanned.kind() == item_kind::text_string));
        require(!view_ok || (copy_ok && text_copy == std::string(text_view.data(), text_view.size())));

        jsoncons::span<const uint8_t> bytes_view;
        std::vector<uint8_t> bytes_copy;
        const bool bytes_view_ok = scanned.bytes(bytes_view);
        const bool bytes_copy_ok = scanned.bytes(bytes_copy);
        require(bytes_copy_ok == (scanned.kind() == item_kind::byte_string));
        require(!bytes_view_ok || (bytes_copy_ok && bytes_copy.size() == bytes_view.size() &&
            (bytes_view.empty() || std::memcmp(bytes_copy.data(), bytes_view.data(), bytes_view.size()) == 0)));

        if (scanned.indefinite() && (copy_ok || bytes_copy_ok))
        {
            std::size_t total = 0;
            for (const item chunk : scanned.children())
            {
                total += static_cast<std::size_t>(chunk.argument());
            }
            require(total == (copy_ok ? text_copy.size() : bytes_copy.size()));
        }

        require(validate_text(scanned) ? scanned.kind() == item_kind::text_string : true);

        exercise_children(scanned, budget);
    }

    void exercise_input(byte_span input, int depth)
    {
        auto scanned = scan(input, depth);
        auto exact = parse_item(input, depth);

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
        }
        else
        {
            require(!exact.has_value());
            require(exact.error().code == scanned.error().code);
            require(exact.error().offset == scanned.error().offset);
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

    for (int depth : depths)
    {
        exercise_input(input, depth);
        exercise_input(byte_span(base, mid), depth);
        exercise_input(byte_span(base + mid, size - mid), depth);
        exercise_input(byte_span(base + p0, size - p0), depth);
    }

    return 0;
}
