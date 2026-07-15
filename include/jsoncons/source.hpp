// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_SOURCE_HPP
#define JSONCONS_SOURCE_HPP

#include <cstdint>
#include <cstring> // std::memcpy
#include <exception>
#include <functional>
#include <istream>
#include <iterator>
#include <memory> // std::addressof
#include <string>
#include <type_traits> // std::enable_if
#include <vector>

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/utility/byte_string.hpp> // jsoncons::byte_traits
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/utility/more_type_traits.hpp>

namespace jsoncons { 

    template <typename Source>
    struct source_reader;

    // The source data must be padded by at least `buffer_padding_size` bytes.
    JSONCONS_INLINE_CONSTEXPR uint8_t buffer_padding_size = 4;

    template <typename CharT>
    class basic_null_istream : public std::basic_istream<CharT>
    {
        class null_buffer : public std::basic_streambuf<CharT>
        {
        public:
            using typename std::basic_streambuf<CharT>::int_type;
            using typename std::basic_streambuf<CharT>::traits_type;

            null_buffer() = default;
            null_buffer(const null_buffer&) = delete;
            null_buffer(null_buffer&&) = default;

            null_buffer& operator=(const null_buffer&) = delete;
            null_buffer& operator=(null_buffer&&) = default;

            int_type overflow( int_type ch = typename std::basic_streambuf<CharT>::traits_type::eof()) override
            {
                return ch;
            }
        } nb_;
    public:
        basic_null_istream()
          : std::basic_istream<CharT>(&nb_)
        {
        }

        basic_null_istream(const null_buffer&) = delete;
        basic_null_istream& operator=(const null_buffer&) = delete;
        basic_null_istream(basic_null_istream&&) noexcept
            : std::basic_istream<CharT>(&nb_)
        {
        }
        basic_null_istream& operator=(basic_null_istream&&) noexcept
        {
            return *this;
        }
    };

    template <typename CharT>
    struct char_result
    {
        CharT value;
        bool eof;
    };

    // text sources

    template <typename CharT,typename Allocator = std::allocator<CharT>>
    class stream_source
    {
    public:
        using value_type = CharT;
        static constexpr std::size_t default_max_buffer_size = 16384;
    private:
        using char_type = typename std::conditional<sizeof(CharT) == sizeof(char),char,CharT>::type;
        using char_allocator_type = typename std::allocator_traits<Allocator>:: template rebind_alloc<value_type>;
        using stream_source_type = stream_source<CharT,Allocator>;

        Allocator alloc_;
        basic_null_istream<char_type> null_is_;
        std::basic_istream<char_type>* stream_ptr_;
        std::basic_streambuf<char_type>* sbuf_;
        std::size_t position_{0};
        value_type* chunk_{nullptr};
        std::size_t chunk_size_{0};
        value_type* data_{nullptr};
        std::size_t length_{0};
    public:

        const Allocator& get_allocator() const
        {
            return alloc_;
        }

        stream_source(const Allocator& alloc = Allocator())
            : alloc_(alloc), stream_ptr_(&null_is_), sbuf_(null_is_.rdbuf())
        {
        }

        // Noncopyable 
        stream_source(const stream_source&) = delete;

        stream_source(stream_source&& other) noexcept
            : alloc_(other.alloc_), stream_ptr_(&null_is_), sbuf_(null_is_.rdbuf())
        {
            chunk_ = other.chunk_;
            data_ = other.data_;
            chunk_size_ = other.chunk_size_;
            length_ = other.length_;
            other.chunk_ = nullptr;
            other.data_ = nullptr;
            other.chunk_size_ = 0;
            other.length_ = 0;

            if (other.stream_ptr_ != &other.null_is_)
            {
                stream_ptr_ = other.stream_ptr_;
                sbuf_ = other.sbuf_;
                position_ = other.position_;
                other.stream_ptr_ = &other.null_is_;
                other.sbuf_ = other.null_is_.rdbuf();
                other.position_ = 0;
            }
        }

        stream_source(stream_source&& other, const Allocator& alloc) noexcept
            : alloc_(alloc), stream_ptr_(&null_is_), sbuf_(null_is_.rdbuf()),
              chunk_size_(other.chunk_size_), length_(other.length_)
        {
            if (alloc == other.get_allocator())
            {
                chunk_ = other.chunk_;
                data_ = other.data_;
                length_ = other.length_;
                other.chunk_ = nullptr;
                other.data_ = nullptr;
                other.length_ = 0;
            }
            else if (other.chunk_ != nullptr)
            {
                chunk_ = std::allocator_traits<char_allocator_type>::allocate(alloc_, chunk_size_);
                data_ = chunk_ + (other.data_ - other.chunk_);
                std::memcpy(data_, other.data_, sizeof(value_type)*other.length_);
            }
            if (other.stream_ptr_ != &other.null_is_)
            {
                stream_ptr_ = other.stream_ptr_;
                sbuf_ = other.sbuf_;
                position_ = other.position_;
            }
            else
            {
                stream_ptr_ = &null_is_;
                sbuf_ = null_is_.rdbuf();
                position_ = 0;
            }
        }

        stream_source(std::basic_istream<char_type>& is,
            const Allocator& alloc = Allocator())
            : alloc_(alloc), stream_ptr_(std::addressof(is)), sbuf_(is.rdbuf()),
              chunk_size_(default_max_buffer_size)
        {
            chunk_ = std::allocator_traits<char_allocator_type>::allocate(alloc_, chunk_size_);
            data_ = chunk_;
        }

        stream_source(std::basic_istream<char_type>& is, std::size_t buf_size,
            const Allocator& alloc = Allocator())
            : alloc_(alloc), stream_ptr_(std::addressof(is)), sbuf_(is.rdbuf()),
              chunk_size_(buf_size)
        {
            chunk_ = std::allocator_traits<char_allocator_type>::allocate(alloc_, chunk_size_);
            data_ = chunk_;
        }

        ~stream_source() noexcept
        {
            if (chunk_)
            {
                std::allocator_traits<char_allocator_type>::deallocate(alloc_, chunk_, chunk_size_);
            }
        }

        stream_source& operator=(const stream_source&) = delete;
      
        void move_assignment(std::true_type, // propagate_on_container_move_assignment
            stream_source&& other) noexcept
        {
            auto alloc = other.alloc_;
            other.alloc_ = alloc_;
            alloc_ = alloc;
            std::swap(chunk_, other.chunk_);
            std::swap(chunk_size_, other.chunk_size_);
            std::swap(data_, other.data_);
            std::swap(length_, other.length_);
            if (other.stream_ptr_ != &other.null_is_)
            {
                stream_ptr_ = other.stream_ptr_;
                sbuf_ = other.sbuf_;
                position_ = other.position_;
            }
            else
            {
                stream_ptr_ = &null_is_;
                sbuf_ = null_is_.rdbuf();
                position_ = 0;
            }
        }

        void move_assignment(std::false_type, // not propagate_on_container_move_assignment
            stream_source&& other) noexcept
        {
            chunk_size_ = other.chunk_size_;
            chunk_ = std::allocator_traits<char_allocator_type>::allocate(alloc_, chunk_size_);
            data_ = chunk_ + (other.data_ - other.chunk_);
            length_ = other.length_;
            std::memcpy(chunk_, other.chunk_, sizeof(value_type)*other.length_);
            if (other.stream_ptr_ != &other.null_is_)
            {
                stream_ptr_ = other.stream_ptr_;
                sbuf_ = other.sbuf_;
                position_ = other.position_;
            }
            else
            {
                stream_ptr_ = &null_is_;
                sbuf_ = null_is_.rdbuf();
                position_ = 0;
            }
        }

        stream_source& operator=(stream_source&& other) noexcept
        {
            move_assignment(typename std::allocator_traits<char_allocator_type>::propagate_on_container_move_assignment(),
                std::move(other));
            return *this;
        }

        const value_type* buffer() const
        {
            return chunk_;
        }

        std::size_t buffer_size() const
        {
            return chunk_size_;
        }

        const value_type* data() const
        {
            return data_;
        }

        std::size_t length() const
        {
            return length_;
        }

        bool eof() const
        {
            return length_ == 0 && stream_ptr_->eof();
        }

        bool is_error() const
        {
            return stream_ptr_->bad();  
        }

        std::size_t position() const
        {
            return position_;
        }

        void ignore(std::size_t length)
        {
            std::size_t len = 0;
            if (length_ > 0)
            {
                len = (std::min)(length_, length);
                position_ += len;
                data_ += len;
                length_ -= len;
            }
            while (len < length)
            {
                fill_buffer();
                if (length_ == 0)
                {
                    break;
                }
                std::size_t len2 = (std::min)(length_, length-len);
                position_ += len2;
                data_ += len2;
                length_ -= len2;
                len += len2;
            }
        }

        char_result<value_type> peek() 
        {
            if (length_ == 0)
            {
                fill_buffer();
            }
            if (length_ > 0)
            {
                value_type c = *data_;
                return char_result<value_type>{c, false};
            }
            else
            {
                return char_result<value_type>{0, true};
            }
        }

        span<const value_type> read_buffer() 
        {
            if (length_ == 0)
            {
                fill_buffer();
            }
            const value_type* data = data_;
            std::size_t length = length_;
            data_ += length_;
            position_ += length_;
            length_ = 0;

            return span<const value_type>(data, length);
        }

        template <typename Buffer>
        span<const value_type> read_span(std::size_t length, Buffer&& buffer)
        {
            if (JSONCONS_UNLIKELY(length == 0))
            {
                return span<const value_type>(data_, std::size_t(0));
            }
            if (length > length_)
            {
                buffer.clear();
                source_reader<stream_source_type>::read(*this, std::forward<Buffer>(buffer), length);
                return span<const value_type>(reinterpret_cast<const value_type*>(buffer.data()), buffer.size());
            }

            const value_type* data = data_;
            data_ += length;
            length_ -= length;
            position_ += length;
            return span<const value_type>(data, length);
        }

        std::size_t read(value_type* p, std::size_t length)
        {
            std::size_t len = 0;
            if (length_ > 0)
            {
                len = (std::min)(length_, length);
                std::memcpy(p, data_, len*sizeof(value_type));
                data_ += len;
                length_ -= len;
                position_ += len;
            }
            if (length - len == 0)
            {
                return len;
            }
            else if (length - len < chunk_size_)
            {
                fill_buffer();
                if (length_ > 0)
                {
                    std::size_t len2 = (std::min)(length_, length-len);
                    std::memcpy(p+len, data_, len2*sizeof(value_type));
                    data_ += len2;
                    length_ -= len2;
                    position_ += len2;
                    len += len2;
                }
                return len;
            }
            else
            {
                if (stream_ptr_->eof())
                {
                    length_ = 0;
                    return 0;
                }
                JSONCONS_TRY
                {
                    std::streamsize count = sbuf_->sgetn(reinterpret_cast<char_type*>(p+len), length-len);
                    std::size_t len2 = static_cast<std::size_t>(count);
                    if (len2 < length-len)
                    {
                        stream_ptr_->clear(stream_ptr_->rdstate() | std::ios::eofbit);
                    }
                    len += len2;
                    position_ += len2;
                    return len;
                }
                JSONCONS_CATCH(const std::exception&)     
                {
                    stream_ptr_->clear(stream_ptr_->rdstate() | std::ios::badbit | std::ios::eofbit);
                    return 0;
                }
            }
        }
    private:
        void reserve_buffer(std::size_t capacity)
        {
            if (capacity <= chunk_size_)
            {
                return;
            }

            value_type* new_buffer = std::allocator_traits<char_allocator_type>::allocate(alloc_, capacity);
            if (length_ > 0)
            {
                std::memcpy(new_buffer, data_, sizeof(value_type)*length_);
            }
            if (chunk_)
            {
                std::allocator_traits<char_allocator_type>::deallocate(alloc_, chunk_, chunk_size_);
            }
            chunk_ = new_buffer;
            chunk_size_ = capacity;
            data_ = chunk_;
        }

        void fill_buffer()
        {
            if (stream_ptr_->eof())
            {
                length_ = 0;
                return;
            }

            data_ = chunk_;
            JSONCONS_TRY
            {
                std::streamsize count = sbuf_->sgetn(reinterpret_cast<char_type*>(chunk_), chunk_size_);
                length_ = static_cast<std::size_t>(count);

                if (length_ < chunk_size_)
                {
                    stream_ptr_->clear(stream_ptr_->rdstate() | std::ios::eofbit);
                }
            }
            JSONCONS_CATCH(const std::exception&)     
            {
                stream_ptr_->clear(stream_ptr_->rdstate() | std::ios::badbit | std::ios::eofbit);
                length_ = 0;
            }
        }
    };

    template <typename CharT,typename Allocator>
    constexpr std::size_t stream_source<CharT,Allocator>::default_max_buffer_size;

    // string_source

    template <typename CharT>
    class string_source 
    {
    public:
        using value_type = CharT;
        using string_view_type = jsoncons::basic_string_view<value_type>;
    private:
        const value_type* data_{nullptr};
        const value_type* current_{nullptr};
        const value_type* end_{nullptr};
    public:
        string_source() noexcept = default;

        // Noncopyable 
        string_source(const string_source&) = delete;

        string_source(string_source&& other) = default;

        template <typename Sourceable>
        string_source(const Sourceable& s,
                      typename std::enable_if<ext_traits::is_sequence_of<Sourceable,value_type>::value>::type* = 0)
            : data_(s.data()), current_(s.data()), end_(s.data()+s.size())
        {
        }

        string_source(const value_type* data)
            : data_(data), current_(data), end_(data+std::char_traits<value_type>::length(data))
        {
        }

        string_source& operator=(const string_source&) = delete;
        string_source& operator=(string_source&& other) = default;

        bool eof() const
        {
            return current_ == end_;  
        }

        bool is_error() const
        {
            return false;  
        }

        std::size_t position() const
        {
            return (current_ - data_)/sizeof(value_type);
        }

        void ignore(std::size_t count)
        {
            std::size_t len;
            if (std::size_t(end_ - current_) < count)
            {
                len = end_ - current_;
            }
            else
            {
                len = count;
            }
            current_ += len;
        }

        char_result<value_type> peek() 
        {
            return current_ < end_ ? char_result<value_type>{*current_, false} : char_result<value_type>{0, true};
        }

        span<const value_type> read_buffer() 
        {
            const value_type* data = current_;
            std::size_t length = end_ - current_;
            current_ = end_;

            return span<const value_type>(data, length);
        }


        template <typename Buffer>
        span<const value_type> read_span(std::size_t length, Buffer&&)
        {
            if (std::size_t(end_ - current_) < length)
            {
                return span<const value_type>();
            }
            const value_type* data = current_;
            current_ += length;
            return span<const value_type>(data, length);
        }

        std::size_t read(value_type* p, std::size_t length)
        {
            std::size_t len;
            if (std::size_t(end_ - current_) < length)
            {
                len = end_ - current_;
            }
            else
            {
                len = length;
            }
            std::memcpy(p, current_, len*sizeof(value_type));
            current_  += len;
            return len;
        }
    };

    // iterator source

    template <typename IteratorT>
    class iterator_source
    {
    public:
        using value_type = typename std::iterator_traits<IteratorT>::value_type;
    private:
        static constexpr std::size_t default_max_buffer_size = 16384;

        IteratorT current_;
        IteratorT end_;
        std::size_t position_{0};
        std::vector<value_type> chunk_;
        std::size_t buffer_len_{0};

        using difference_type = typename std::iterator_traits<IteratorT>::difference_type;
        using iterator_category = typename std::iterator_traits<IteratorT>::iterator_category;
    public:

        // Noncopyable 
        iterator_source(const iterator_source&) = delete;

        iterator_source(iterator_source&& other) = default;

        iterator_source(const IteratorT& first, const IteratorT& last, std::size_t buf_size = default_max_buffer_size)
            : current_(first), end_(last), chunk_(buf_size)
        {
        }
        
        ~iterator_source() = default;

        iterator_source& operator=(const iterator_source&) = delete;
        iterator_source& operator=(iterator_source&& other) = default;

        bool eof() const
        {
            return !(current_ != end_);  
        }

        bool is_error() const
        {
            return false;  
        }

        std::size_t position() const
        {
            return position_;
        }

        void ignore(std::size_t count)
        {
            while (count-- > 0 && current_ != end_)
            {
                ++position_;
                ++current_;
            }
        }

        char_result<value_type> peek() 
        {
            return current_ != end_ ? char_result<value_type>{*current_, false} : char_result<value_type>{0, true};
        }

        span<const value_type> read_buffer() 
        {
            if (buffer_len_ == 0)
            {
                buffer_len_ = read(chunk_.data(), chunk_.size());
            }
            std::size_t length = buffer_len_;
            buffer_len_ = 0;

            return span<const value_type>(chunk_.data(), length);
        }


        template <typename Buffer>
        span<const value_type> read_span(std::size_t length, Buffer&&)
        {
            if (length > default_max_buffer_size)
            {
                return span<const value_type>();
            }
            if (length > chunk_.size())
            {
                chunk_.resize(length);
            }
            std::size_t actual = read(chunk_.data(), length);
            if (actual != length)
            {
                return span<const value_type>();
            }
            return span<const value_type>(chunk_.data(), length);
        }

        template <typename Category = iterator_category>
        typename std::enable_if<std::is_same<Category,std::random_access_iterator_tag>::value, std::size_t>::type
        read(value_type* data, std::size_t length)
        {
            std::size_t count = (std::min)(length, static_cast<std::size_t>(std::distance(current_, end_)));

            //JSONCONS_COPY(current_, current_ + count, data);

            auto end = current_ + count;
            value_type* p = data;
            while (current_ != end)
            {
                *p++ = *current_++;
            }

            //current_ += count;
            position_ += count;

            return count;
        }

        template <typename Category = iterator_category>
        typename std::enable_if<!std::is_same<Category,std::random_access_iterator_tag>::value, std::size_t>::type
        read(value_type* data, std::size_t length)
        {
            value_type* p = data;
            value_type* pend = data + length;

            while (p < pend && current_ != end_)
            {
                *p = static_cast<value_type>(*current_);
                ++p;
                ++current_;
            }

            position_ += (p - data);

            return p - data;
        }
    };

    // binary sources

    using binary_stream_source = stream_source<uint8_t>;

    class bytes_source 
    {
    public:
        typedef uint8_t value_type;
    private:
        const value_type* data_{nullptr};
        const value_type* current_{nullptr};
        const value_type* end_{nullptr};
    public:
        bytes_source() noexcept = default;

        // Noncopyable 
        bytes_source(const bytes_source&) = delete;

        bytes_source(bytes_source&&) = default;

        template <typename Sourceable>
        bytes_source(const Sourceable& source,
                     typename std::enable_if<ext_traits::is_bytes_view_like<Sourceable>::value,int>::type = 0)
            : data_(reinterpret_cast<const value_type*>(source.data())), 
              current_(data_), 
              end_(data_+source.size())
        {
        }

        bytes_source& operator=(const bytes_source&) = delete;
        bytes_source& operator=(bytes_source&&) = default;

        bool eof() const
        {
            return current_ == end_;  
        }

        bool is_error() const
        {
            return false;  
        }

        std::size_t position() const
        {
            return current_ - data_;
        }

        void ignore(std::size_t count)
        {
            std::size_t len;
            if (std::size_t(end_ - current_) < count)
            {
                len = end_ - current_;
            }
            else
            {
                len = count;
            }
            current_ += len;
        }

        char_result<value_type> peek() 
        {
            return current_ < end_ ? char_result<value_type>{*current_, false} : char_result<value_type>{0, true};
        }

        span<const value_type> read_buffer() 
        {
            const value_type* data = current_;
            std::size_t length = end_ - current_;
            current_ = end_;

            return span<const value_type>(data, length);
        }


        template <typename Buffer>
        span<const value_type> read_span(std::size_t length, Buffer&&)
        {
            if (std::size_t(end_ - current_) < length)
            {
                return span<const value_type>();
            }
            const value_type* data = current_;
            current_ += length;
            return span<const value_type>(data, length);
        }

        std::size_t read(value_type* p, std::size_t length)
        {
            std::size_t len;
            if (std::size_t(end_ - current_) < length)
            {
                len = end_ - current_;
            }
            else
            {
                len = length;
            }
            std::memcpy(p, current_, len*sizeof(value_type));
            current_  += len;
            return len;
        }
    };

    // binary_iterator source

    template <typename IteratorT>
    class binary_iterator_source
    {
    public:
        using value_type = uint8_t;
    private:
        static constexpr std::size_t default_max_buffer_size = 16384;

        IteratorT current_;
        IteratorT end_;
        std::size_t position_{0};
        std::vector<value_type> chunk_;
        std::size_t buffer_len_{0};

        using difference_type = typename std::iterator_traits<IteratorT>::difference_type;
        using iterator_category = typename std::iterator_traits<IteratorT>::iterator_category;
    public:

        // Noncopyable 
        binary_iterator_source(const binary_iterator_source&) = delete;

        binary_iterator_source(binary_iterator_source&& other) = default;

        binary_iterator_source(const IteratorT& first, const IteratorT& last, std::size_t buf_size = default_max_buffer_size)
            : current_(first), end_(last), chunk_(buf_size)
        {
        }

        binary_iterator_source& operator=(const binary_iterator_source&) = delete;
        binary_iterator_source& operator=(binary_iterator_source&& other) = default;

        bool eof() const
        {
            return !(current_ != end_);  
        }

        bool is_error() const
        {
            return false;  
        }

        std::size_t position() const
        {
            return position_;
        }

        void ignore(std::size_t count)
        {
            while (count-- > 0 && current_ != end_)
            {
                ++position_;
                ++current_;
            }
        }

        char_result<value_type> peek() 
        {
            return current_ != end_ ? char_result<value_type>{static_cast<value_type>(*current_), false} : char_result<value_type>{0, true};
        }

        span<const value_type> read_buffer() 
        {
            if (buffer_len_ == 0)
            {
                buffer_len_ = read(chunk_.data(), chunk_.size());
            }
            std::size_t length = buffer_len_;
            buffer_len_ = 0;

            return span<const value_type>(chunk_.data(), length);
        }

        template <typename Buffer>
        span<const value_type> read_span(std::size_t length, Buffer&&)
        {
            if (length > default_max_buffer_size)
            {
                return span<const value_type>();
            }
            if (length > chunk_.size())
            {
                chunk_.resize(length);
            }
            std::size_t actual = read(chunk_.data(), length);
            if (actual != length)
            {
                return span<const value_type>();
            }
            return span<const value_type>(chunk_.data(), length);
        }

        template <typename Category = iterator_category>
        typename std::enable_if<std::is_same<Category,std::random_access_iterator_tag>::value, std::size_t>::type
        read(value_type* data, std::size_t length)
        {
            std::size_t count = (std::min)(length, static_cast<std::size_t>(std::distance(current_, end_)));
            //JSONCONS_COPY(current_, current_ + count, data);

            auto end = current_ + count;
            value_type* p = data;
            while (current_ != end)
            {
                *p++ = *current_++;
            }

            //current_ += count;
            position_ += count;

            return count;
        }

        template <typename Category = iterator_category>
        typename std::enable_if<!std::is_same<Category,std::random_access_iterator_tag>::value, std::size_t>::type
        read(value_type* data, std::size_t length)
        {
            value_type* p = data;
            value_type* pend = data + length;

            while (p < pend && current_ != end_)
            {
                *p = static_cast<value_type>(*current_);
                ++p;
                ++current_;
            }

            position_ += (p - data);

            return p - data;
        }
    };

    template <typename Source>
    struct source_reader
    {
        using value_type = typename Source::value_type;
        static constexpr std::size_t max_buffer_length = 16384;

        template <typename Buffer>
        static
        typename std::enable_if<ext_traits::is_byte<typename Source::value_type>::value &&
            ext_traits::is_byte<typename Buffer::value_type>::value, std::size_t>::type
        read(Source& source, Buffer& v, std::size_t length)
        {
            std::size_t unread = length;

            while (unread > 0 && !source.eof())
            {
                std::size_t n = (std::min)(max_buffer_length, unread);
                std::size_t offset = v.size();
                v.resize(v.size()+n);
                std::size_t actual = source.read(reinterpret_cast<value_type*>(&v[0]) + offset, n);
                unread -= actual;
            }
            return length - unread;
        }
    };
#if __cplusplus >= 201703L
// not needed for C++17
#else
    template <typename Source>
    constexpr std::size_t source_reader<Source>::max_buffer_length;
#endif

} // namespace jsoncons

#endif // JSONCONS_SOURCE_HPP
