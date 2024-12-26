// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_CSV_CSV_READER_HPP
#define JSONCONS_EXT_CSV_CSV_READER_HPP

#include <istream> // std::basic_istream
#include <memory> // std::allocator
#include <stdexcept>
#include <string>
#include <utility> // std::move
#include <vector>

#include <jsoncons/json.hpp>
#include <jsoncons/json_decoder.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_reader.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/source_adaptor.hpp>
#include <jsoncons_ext/csv/csv_error.hpp>
#include <jsoncons_ext/csv/csv_options.hpp>
#include <jsoncons_ext/csv/csv_parser.hpp>

namespace jsoncons { namespace csv {

    template <typename CharT,typename Source=jsoncons::stream_source<CharT>,typename Allocator=std::allocator<char>>
    class basic_csv_reader final  : private chunk_reader<CharT>
    {
        struct stack_item
        {
            stack_item() noexcept
               : array_begun_(false)
            {
            }

            bool array_begun_;
        };
        using char_type = CharT;
        using temp_allocator_type = Allocator;
        using char_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<CharT>;

        basic_csv_reader(const basic_csv_reader&) = delete; 
        basic_csv_reader& operator = (const basic_csv_reader&) = delete; 

        basic_default_json_visitor<CharT> default_visitor_;
        text_source_adaptor<Source> source_;
        basic_json_visitor<CharT>& visitor_;
        basic_csv_parser<CharT,Allocator> parser_;

    public:
        // Structural characters
        static constexpr size_t default_max_buffer_size = 16384;
        //!  Parse an input stream of CSV text into a json object
        /*!
          \param is The input stream to read from
        */

        template <typename Sourceable>
        basic_csv_reader(Sourceable&& source,
                         basic_json_visitor<CharT>& visitor, 
                         const Allocator& alloc = Allocator())

           : basic_csv_reader(std::forward<Sourceable>(source), 
                              visitor, 
                              basic_csv_decode_options<CharT>(), 
                              default_csv_parsing(), 
                              alloc)
        {
        }

        template <typename Sourceable>
        basic_csv_reader(Sourceable&& source,
                         basic_json_visitor<CharT>& visitor,
                         const basic_csv_decode_options<CharT>& options, 
                         const Allocator& alloc = Allocator())

            : basic_csv_reader(std::forward<Sourceable>(source), 
                               visitor, 
                               options, 
                               default_csv_parsing(),
                               alloc)
        {
        }

        template <typename Sourceable>
        basic_csv_reader(Sourceable&& source,
                         basic_json_visitor<CharT>& visitor,
                         std::function<bool(csv_errc,const ser_context&)> err_handler, 
                         const Allocator& alloc = Allocator())
            : basic_csv_reader(std::forward<Sourceable>(source), 
                               visitor, 
                               basic_csv_decode_options<CharT>(), 
                               err_handler,
                               alloc)
        {
        }

        template <typename Sourceable>
        basic_csv_reader(Sourceable&& source,
                         basic_json_visitor<CharT>& visitor,
                         const basic_csv_decode_options<CharT>& options,
                         std::function<bool(csv_errc,const ser_context&)> err_handler, 
                         const Allocator& alloc = Allocator())
           : source_(std::forward<Sourceable>(source)),
             visitor_(visitor),
             parser_(this, options, err_handler, alloc)
             
        {
        }

        ~basic_csv_reader() noexcept = default;

        void read()
        {
            std::error_code ec;
            read(ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec,parser_.line(),parser_.column()));
            }
        }

        void read(std::error_code& ec)
        {
            if (source_.is_error())
            {
                ec = csv_errc::source_error;
                return;
            }   
            if (parser_.source_exhausted())
            {
                auto s = source_.read_buffer(ec);
                if (ec) return;
                if (s.size() > 0)
                {
                    parser_.set_buffer(s.data(),s.size());
                }
            }
            parser_.parse_some(visitor_, ec);
        }

        std::size_t line() const
        {
            return parser_.line();
        }

        std::size_t column() const
        {
            return parser_.column();
        }

        bool eof() const
        {
            return parser_.source_exhausted() && source_.eof();
        }

    private:

        bool read_chunk(basic_parser_input<char_type>&, std::error_code& ec) final
        {
            //std::cout << "UPDATE BUFFER\n";
            bool success = false;
            auto s = source_.read_buffer(ec);
            if (ec) return false;
            if (s.size() > 0)
            {
                parser_.set_buffer(s.data(),s.size());
                success = true;
            }

            return success;
        }
    };

    using csv_string_reader = basic_csv_reader<char,string_source<char>>;
    using wcsv_string_reader = basic_csv_reader<wchar_t,string_source<wchar_t>>;
    using csv_stream_reader = basic_csv_reader<char,stream_source<char>>;
    using wcsv_stream_reader = basic_csv_reader<wchar_t,stream_source<wchar_t>>;

}}

#endif // JSONCONS_EXT_CSV_CSV_READER_HPP
