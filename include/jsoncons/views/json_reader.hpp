// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons2 for latest version

#ifndef JSONCONS2_JSON_READER_HPP
#define JSONCONS2_JSON_READER_HPP

#include <jsoncons/views/json_element.hpp>
#include <jsoncons/views/read_json.hpp>
#include <string_view>
#include <iterator>
#include <stdexcept>
#include <limits>
#include <cmath>
#include <vector>

namespace jsoncons2 {

    inline constexpr std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)();

    enum class json_event_kind : uint8_t
    {
        none = 0,
        end_of_input,
        raw,
        null,
        boolean,
        number,
        string,
        start_array,
        end_array,
        start_object,
        end_object,
        key
    };

    class json_reader
    {
    public:
        using allocator_type = std::allocator<uint8_t>;
    private:
        allocator_type alloc_;
        
        struct stack_item
        {
            json_event_kind event_kind;
            bool empty;
        };
        
        uint8_t* hdr_;
        std::size_t hdr_capacity_;
        uint8_t* end_;
        uint8_t* ptr_;
        // Root value of the document
        std::size_t length_;
        read_json_flags flags_;
        bool raw_;
        bool inv_;
        uint8_t* raw_end_;
        uint8_t** pre_;
        // Allocator used by document (nonnull)
        json_event_kind event_kind_;
        json_element current_;
        std::vector<stack_item> stack_;       

    public:
        json_reader(std::string_view input, std::error_code& ec);
        
        ~json_reader() noexcept
        {
            if (hdr_ != nullptr)
            {
                std::allocator_traits<allocator_type>::deallocate(alloc_, hdr_, hdr_capacity_);
                hdr_ = nullptr;
            }
        }
        
        json_event_kind event_kind() const noexcept
        {
            return event_kind_;
        }

        json_type type() const noexcept
        {
            return current_.type();
        }

        std::string_view get_string_view() const
        {
            return current_.get_string_view();
        }

        bool get_bool() const
        {
            return current_.get_bool();
        }
        
        template <typename T>
        constexpr T cast() const
        {
            return current_.template cast<T>();
        }
        
        bool done() const
        {
            return event_kind_ == json_event_kind::end_of_input;
        }
        
        void next(std::error_code& ec);
                
        json_element read_element(std::error_code& ec);
    };

} // jsoncons2

#endif

