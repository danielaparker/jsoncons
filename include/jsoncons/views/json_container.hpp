// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_DOCUMENT_HPP
#define JSONCONS_JSON_DOCUMENT_HPP

#include <jsoncons/views/json_view.hpp>
#include <jsoncons/views/read_json.hpp>
#include <jsoncons/views/deserialize_result.hpp>
#include <string_view>
#include <iterator>
#include <stdexcept>

namespace jsoncons {

    class json_container
    {
    public:
        using allocator_type = std::allocator<uint8_t>;
    private:
        using element_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<json_ref>;
        json_ref* root_;
        std::size_t root_capacity_;
        // The total number of bytes read when parsing JSON (nonzero). 
        size_t dat_read_;
        // The total number of value read when parsing JSON (nonzero). 
        size_t val_read_;
        // The string pool used by JSON values (nullable). 
        uint8_t* hdr_;
        std::size_t hdr_capacity_;
        allocator_type alloc_;
        element_allocator_type element_alloc_;
    public:
        json_container()
            : root_(nullptr), root_capacity_(0), dat_read_(0),
            val_read_(0), hdr_(nullptr), hdr_capacity_(0)
        {
        }
        json_container(json_ref* root, std::size_t root_capacity,
            size_t dat_read, size_t val_read, uint8_t* hdr, std::size_t hdr_capacity, 
            const allocator_type& alloc, const element_allocator_type& element_alloc)
            : root_(root), root_capacity_(root_capacity), dat_read_(dat_read),
            val_read_(val_read), hdr_(hdr), hdr_capacity_(0), 
            alloc_{alloc}, element_alloc_(element_alloc)
        {
        }
        json_container(const json_container& other) = delete;
        json_container(json_container&& other) noexcept
            : root_(nullptr), dat_read_(0), val_read_(0), hdr_(nullptr), hdr_capacity_(0)
        {
            std::swap(root_, other.root_);
            std::swap(dat_read_, other.dat_read_);
            std::swap(val_read_, other.val_read_);
            std::swap(hdr_, other.hdr_);
            std::swap(hdr_capacity_, other.hdr_capacity_);
        }
        json_container& operator=(const json_container& other) = delete;

        json_container& operator=(json_container&& other) noexcept
        {
            std::swap(root_, other.root_);
            std::swap(dat_read_, other.dat_read_);
            std::swap(val_read_, other.val_read_);
            std::swap(hdr_, other.hdr_);
            std::swap(hdr_capacity_, other.hdr_capacity_);
            return *this;
        }

        ~json_container()
        {
            if (hdr_ != nullptr)
            {
                std::allocator_traits<allocator_type>::deallocate(alloc_, (uint8_t*)hdr_, hdr_capacity_);
                hdr_ = nullptr;
            }
            if (root_ != nullptr)
            {
                std::allocator_traits<element_allocator_type>::deallocate(element_alloc_, root_, root_capacity_);
                root_ = nullptr;
            }
        }

        json_view root()
        {
            return json_view(root_);
        }

        static deserialize_result<json_container> parse(std::string_view sv, read_json_flags flg = read_json_flags::none)
        {
            allocator_type alloc{};
            element_allocator_type element_alloc{};
            flg &= ~read_json_flags::insitu; /* const string cannot be modified */
            return parse(const_cast<char*>(sv.data()), sv.size(), flg, alloc, element_alloc);
        }

        static deserialize_result<json_container> parse_file(std::string_view sv, read_json_flags flg = read_json_flags::none)
        {
            allocator_type alloc{};
            element_allocator_type element_alloc{};
            flg &= ~read_json_flags::insitu; /* const string cannot be modified */
            return yyjson_read_file(const_cast<char*>(sv.data()), flg, alloc, element_alloc);
        }

        std::size_t read_size() const
        {
            return dat_read_;
        }

        std::size_t value_count() const
        {
            return val_read_;
        }
    private:
        static deserialize_result<json_container> parse(char* dat, size_t len, read_json_flags flg,
            allocator_type& alloc, element_allocator_type& element_alloc);

        static deserialize_result<json_container> yyjson_read_opts(char* dat,
            std::size_t len,
            read_json_flags flg,
            allocator_type& alloc, element_allocator_type& element_alloc);

        static deserialize_result<json_container> yyjson_read_file(const char* path,
            read_json_flags flg,
            allocator_type& alloc, element_allocator_type& element_alloc);

        static deserialize_result<json_container> yyjson_read_fp(FILE* file,
            read_json_flags flg,
            allocator_type& alloc, element_allocator_type& element_alloc);
        
        static deserialize_result<json_container> read_root_single(uint8_t *hdr,
            std::size_t hdr_capacity,
            uint8_t *cur,
            uint8_t *end,
            read_json_flags flags,
            allocator_type& alloc, element_allocator_type& element_alloc);
        
        static deserialize_result<json_container> read_root_minify(uint8_t *hdr,
            std::size_t hdr_capacity,
            uint8_t *cur,
            uint8_t *end,
            read_json_flags flags,
            allocator_type& alloc, element_allocator_type& element_alloc);
        
        static deserialize_result<json_container> read_root_pretty(uint8_t *hdr,
            std::size_t hdr_capacity,
            uint8_t *cur,
            uint8_t *end,
            read_json_flags flags,
            allocator_type& alloc, element_allocator_type& element_alloc);
    };

} // jsoncons

#endif

