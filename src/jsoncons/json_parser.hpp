// Copyright 2015 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_PARSER_HPP
#define JSONCONS_JSON_PARSER_HPP

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <stdexcept>
#include <system_error>
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json_input_handler.hpp"
#include "jsoncons/parse_error_handler.hpp"

namespace jsoncons {

namespace mode {
enum mode_t {
    array,
    done,
    key,
    object
};
};

enum actions {
    T4 = -10,
    F5 = -11,
    N4 = -12,
    S2 = -13,
    S3 = -14,
    S4 = -15,
    S5 = -16,
    Sb = -17,
    Sf = -18,
    Sn = -19,
    Sr = -20,
    St = -21,
    U4 = -22,
    U10 = -23,
    C4 = -24
};

enum states {
    GO,  /* start    */
    OK,  /* ok       */
    OB,  /* object   */
    KE,  /* key      */
    CO,  /* colon    */
    VA,  /* value    */
    AR,  /* array    */
    ST,  /* string   */
    ES,  /* escape   */
    U0,  /* u1       */
    U1,  /* u2       */
    U2,  /* u3       */
    U3,  /* u4       */
    SP,  /* SP       */
    U5,  /* u5       */
    U6,  /* u6       */
    U7,  /* u7       */
    U8,  /* u8       */
    U9,  /* u9       */
    MI,  /* minus    */
    ZE,  /* zero     */
    IN,  /* integer  */
    FR,  /* fraction */
    E1,  /* e        */
    E2,  /* ex       */
    E3,  /* exp      */
    T1,  /* tr       */
    T2,  /* tru      */
    T3,  /* true     */
    F1,  /* fa       */
    F2,  /* fal      */
    F3,  /* fals     */
    F4,  /* false    */
    N1,  /* nu       */
    N2,  /* nul      */
    N3,  /* null     */
    C1,  // /
    C2,  // /*
    C3,  // *
    state_count
};

#define __   -1     /* the universal error code */

/*
    Characters are mapped into these 31 character classes. This allows for
    a significant reduction in the size of the state transition table.
*/

namespace input {
enum input_t {
    space,  /* space */
    lf, // Linefeed
    cr, // Carriage return
    white,  /* other whitespace */
    lcurb,  /* {  */
    rcurb,  /* } */
    lsqrb,  /* [ */
    rsqrb,  /* ] */
    c_colon,  /* : */
    c_comma,  /* , */
    quote,  /* " */
    backs,  /* \ */
    slash,  /* / */
    star,   /* * */
    plus,   /* + */
    minus,  /* - */
    point,  /* . */
    zero,  /* 0 */
    digit,  /* 123456789 */
    a,  /* a */
    b,  /* b */
    c,  /* c */
    d,  /* d */
    e,  /* e */
    f,  /* f */
    l,  /* l */
    n,  /* n */
    r,  /* r */
    s,  /* s */
    t,  /* t */
    u,  /* u */
    ABCDF,  /* ABCDF */
    E,      /* E */
    etc,    /* everything else */
    input_count
};
};

template<typename Char>
class basic_json_reader : private basic_parsing_context<Char>
{
    struct stack_item
    {
        stack_item()
           : mode(0),
             minimum_structure_capacity(0)
        {
        }
        int mode;
        size_t minimum_structure_capacity;
    };
    static int state_transition_table[state_count][input::input_count];
    static int ascii_class[128];
    static const size_t default_max_buffer_length = 16384;
public:
    basic_json_reader(std::basic_istream<Char>& is,
                      basic_json_input_handler<Char>& handler)
       : top_(-1),
         stack_(100),
         is_(std::addressof(is)),
         handler_(std::addressof(handler)),
         err_handler_(std::addressof(default_basic_parse_error_handler<Char>::instance())),
         buffer_capacity_(default_max_buffer_length),
         is_negative_(false),
         cp_(0)

    {
        state_ = GO;
        this->depth_ = 200;
    }

    basic_json_reader(std::basic_istream<Char>& is,
                      basic_json_input_handler<Char>& handler,
                      basic_parse_error_handler<Char>& err_handler)
       : top_(-1),
         stack_(100),
         is_(std::addressof(is)),
         handler_(std::addressof(handler)),
         err_handler_(std::addressof(err_handler)),
         buffer_capacity_(default_max_buffer_length),
         is_negative_(false),
         cp_(0)

    {
        state_ = GO;
        this->depth_ = 200;
    }

    ~basic_json_reader()
    {
    }

    void end_frac_value()
    {
        try
        {
            double d = string_to_float(string_buffer_);
            if (is_negative_)
                d = -d;
            handler_->value(d, *this);
        }
        catch (...)
        {
            err_handler_->error(std::error_code(json_parser_errc::invalid_number, json_parser_category()), *this);
            handler_->value(null_type(), *this);
        }
        string_buffer_.clear();
        is_negative_ = false;
        state_ = OK;
    }

    void end_integer_value()
    {
        if (is_negative_)
        {
            try
            {
                long long d = string_to_integer(is_negative_, string_buffer_.c_str(), string_buffer_.length());
                handler_->value(d, *this);
            }
            catch (const std::exception&)
            {
                try
                {
                    double d = string_to_float(string_buffer_);
                    handler_->value(-d, *this);
                }
                catch (...)
                {
                    err_handler_->error(std::error_code(json_parser_errc::invalid_number, json_parser_category()), *this);
                    handler_->value(null_type(), *this);
                }
            }
        }
        else
        {
            try
            {
                unsigned long long d = string_to_unsigned(string_buffer_.c_str(), string_buffer_.length());
                handler_->value(d, *this);
            }
            catch (const std::exception&)
            {
                try
                {
                    double d = string_to_float(string_buffer_);
                    handler_->value(d, *this);
                }
                catch (...)
                {
                    err_handler_->error(std::error_code(json_parser_errc::invalid_number, json_parser_category()), *this);
                    handler_->value(null_type(), *this);
                }
            }
        }
        string_buffer_.clear();
        is_negative_ = false;
        state_ = OK;
    }

    void pop_key() // -9
    {
        if (!pop(mode::key))
        {
            err_handler_->error(std::error_code(json_parser_errc::expected_container, json_parser_category()), *this);
        }
        handler_->end_object(*this);
        state_ = OK;
    }

    void pop_object() // -8
    {
        if (!pop(mode::object))
        {
            err_handler_->error(std::error_code(json_parser_errc::expected_container, json_parser_category()), *this);
        }
        handler_->end_object(*this);
        state_ = OK;
    }

    void pop_array() // -7
    {
        if (!pop(mode::array))
        {
            err_handler_->error(std::error_code(json_parser_errc::expected_container, json_parser_category()), *this);
        }
        handler_->end_array(*this);
        state_ = OK;
    }

    void push_key(size_t i) // -6
    {
        if (!push(mode::key))
        {
            err_handler_->error(std::error_code(json_parser_errc::expected_container, json_parser_category()), *this);
        }
        state_ = OB;
        //count_members(i+1);
        handler_->begin_object(*this);
    }

    void push_array(size_t i) // -5
    {
        if (!push(mode::array))
        {
            err_handler_->error(std::error_code(json_parser_errc::expected_container, json_parser_category()), *this);
        }
        state_ = AR;
        count_members(i + 1);
        handler_->begin_array(*this);
    }

    void append_codepoint(int next_input,int next_char)
    {
        switch (next_input)
        {
        case '0': 
        case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
        case input::a:  /* a */
        case input::b:  /* b */
        case input::c:  /* c */
        case input::d:  /* d */
        case input::e:  /* e */
        case input::f:  /* f */
        case input::ABCDF:  /* ABCDF */
        case input::E:      /* E */
            append_to_codepoint(next_char);
            break;
        default:
            err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
            break;
        }
    }

    void append_second_codepoint(int next_input,int next_char)
    {
        switch (next_input)
        {
        case '0': 
        case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
        case input::a:  /* a */
        case input::b:  /* b */
        case input::c:  /* c */
        case input::d:  /* d */
        case input::e:  /* e */
        case input::f:  /* f */
        case input::ABCDF:  /* ABCDF */
        case input::E:      /* E */
            append_to_codepoint(next_char);
            break;
        default:
            err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
            break;
        }
    }

    void escape_next_char(int next_input)
    {
        switch (next_input)
        {
        case input::quote:  /* " */
            string_buffer_.push_back('\"');
            state_ = ST;
            break;
        case input::backs:  /* \ */
            string_buffer_.push_back('\\');
            state_ = ST;
            break;
        case input::slash:  /* / */
            string_buffer_.push_back('/');
            state_ = ST;
            break;
        case input::b:  /* b */
            string_buffer_.push_back('\b');
            state_ = ST;
            break;
        case input::f:  /* f */
            string_buffer_.push_back('\f');
            state_ = ST;
            break;
        case input::n:  /* n */
            string_buffer_.push_back('\n');
            state_ = ST;
            break;
        case input::r:  /* r */
            string_buffer_.push_back('\r');
            state_ = ST;
            break;
        case input::t:  /* t */
            string_buffer_.push_back('\t');
            state_ = ST;
            break;
        case input::u:  /* u */
            cp_ = 0;
            state_ = U1;
            break;
        default:    /* everything else */
            err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
            break;
        }
    }

    void end_string_value() // " -4 
    {
        switch (stack_[top_].mode)
        {
        case mode::key:
            handler_->name(string_buffer_.c_str(), string_buffer_.length(), *this);
            string_buffer_.clear();
            state_ = CO;
            break;
        case mode::array:
        case mode::object:
            handler_->value(string_buffer_.c_str(), string_buffer_.length(), *this);
            string_buffer_.clear();
            state_ = OK;
            break;
        default:
            err_handler_->error(std::error_code(json_parser_errc::expected_container, json_parser_category()), *this);
            break;
        }
        string_buffer_.clear();
        is_negative_ = false;
    }

    void flip_object_key() // , -3
    {
        switch (stack_[top_].mode)
        {
        case mode::object:
            // A comma causes a flip from object mode to key mode.
            if (!flip(mode::object, mode::key))
            {
                err_handler_->error(std::error_code(json_parser_errc::expected_container, json_parser_category()), *this);
            }
            //handler_->end_object(*this);
            state_ = KE;
            break;
        case mode::array:
            state_ = VA;
            break;
        default:
            err_handler_->error(std::error_code(json_parser_errc::expected_container, json_parser_category()), *this);
            break;
        }
    }

    void flip_key_object()
    {
        if (!flip(mode::key, mode::object))
        {
            err_handler_->error(std::error_code(json_parser_errc::expected_container, json_parser_category()), *this);
        }
        state_ = VA;
    }
 
    void append_to_codepoint(int next_char)
    {
        cp_ *= 16;
        if (next_char >= '0'  &&  next_char <= '9')
        {
            cp_ += next_char - '0';
        }
        else if (next_char >= 'a'  &&  next_char <= 'f')
        {
            cp_ += next_char - 'a' + 10;
        }
        else if (next_char >= 'A'  &&  next_char <= 'F')
        {
            cp_ += next_char - 'A' + 10;
        }
        else
        {
            err_handler_->error(std::error_code(json_parser_errc::invalid_hex_escape_sequence, json_parser_category()), *this);
        }
    }

    bool eof() const
    {
        return eof_;
    }

    unsigned long do_line_number() const override
    {
        return line_;
    }

    unsigned long do_column_number() const override
    {
        return column_;
    }

    bool do_eof() const override
    {
        return eof_;
    }

    Char do_last_char() const override
    {
        return c_;
    }

    size_t do_minimum_structure_capacity() const override
    {
        return top_ >= 0 ? stack_[top_].minimum_structure_capacity : 0;
    }

    int push(int mode)
    {
        /*
            Push a mode onto the stack. Return false if there is overflow.
        */
        ++top_;
        if (top_ >= depth_)
        {
            depth_ *= 2;
            stack_.resize(depth_);
        }
        stack_[top_].mode = mode;
        return true;
    }

    int flip(int mode1, int mode2)
    {
        /*
            Push a mode onto the stack. Return false if there is overflow.
        */
        if (top_ < 0 || stack_[top_].mode != mode1)
        {
            return false;
        }
        stack_[top_].mode = mode2;
        return true;
    }

    int pop(int mode)
    {
        /*
            Pop the stack, assuring that the current mode matches the expectation.
            Return false if there is underflow or if the modes mismatch.
        */
        if (top_ < 0 || stack_[top_].mode != mode)
        {
            return false;
        }
        --top_;
        return true;
    }

    template<typename Char>
    unsigned long long string_to_unsigned(const Char *s, size_t length) throw(std::overflow_error)
    {
        const unsigned long long max_value = std::numeric_limits<unsigned long long>::max JSONCONS_NO_MACRO_EXP();
        const unsigned long long max_value_div_10 = max_value / 10;
        unsigned long long n = 0;
        for (size_t i = 0; i < length; ++i)
        {
            unsigned long long x = s[i] - '0';
            if (n > max_value_div_10)
            {
                throw std::overflow_error("Unsigned overflow");
            }
            n = n * 10;
            if (n > max_value - x)
            {
                throw std::overflow_error("Unsigned overflow");
            }

            n += x;
        }
        return n;
    }

    template<typename Char>
    long long string_to_integer(bool has_neg, const Char *s, size_t length) throw(std::overflow_error)
    {
        const long long max_value = std::numeric_limits<long long>::max JSONCONS_NO_MACRO_EXP();
        const long long max_value_div_10 = max_value / 10;

        long long n = 0;
        for (size_t i = 0; i < length; ++i)
        {
            long long x = s[i] - '0';
            if (n > max_value_div_10)
            {
                throw std::overflow_error("Integer overflow");
            }
            n = n * 10;
            if (n > max_value - x)
            {
                throw std::overflow_error("Integer overflow");
            }

            n += x;
        }
        return has_neg ? -n : n;
    }

    void read()
    {
        state_ = GO;
        eof_ = false;
        handler_->begin_json();
        buffer_.resize(buffer_capacity_);
        while (!eof_)
        {
            if (!is_->eof())
            {
                is_->read(&buffer_[0], buffer_capacity_);
                buffer_length_ = static_cast<size_t>(is_->gcount());
                if (buffer_length_ == 0)
                {
                    eof_ = true;
                }
            }
            else
            {
                eof_ = true;
            }
            if (!eof_)
            {
                read_buffer();
            }
        }
        check_done();
        handler_->end_json();
    }

    int read_buffer()
    {
        for (size_t i = 0; i < buffer_length_; ++i)
        {
            int next_char = buffer_[i];

            int next_input, next_state;
            /*
                Determine the character's class.
            */
            if (next_char < 0)
            {
                return false;
            }
            if (next_char >= 128)
            {
                next_input = input::etc;
            }
            else
            {
                next_input = ascii_class[next_char];
                if (next_input <= __)
                {
                    return false;
                }
            }
            switch (state_)
            {
            case GO:  /* start    */
                switch (next_input)
                {
                case input::space:  /* space */
                case input::lf: // Linefeed
                case input::cr: // Carriage return
                case input::white:  /* other whitespace */
                    break; // No change
                case input::lcurb:  /* {  */
                    push_key(i);
                    break;
                case input::lsqrb:  /* [ */
                    push_array(i);
                    break;
                case input::slash:  /* / */
                    saved_state_ = state_;
                    state_ = C1;
                    break;
                case input::rcurb:  /* } */
                    err_handler_->error(std::error_code(json_parser_errc::unexpected_end_of_object, json_parser_category()), *this);
                    break;
                case input::rsqrb:  /* ] */
                    err_handler_->error(std::error_code(json_parser_errc::unexpected_end_of_array, json_parser_category()), *this);
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_container, json_parser_category()), *this);
                    break;
                }
                break;

            case OK:  /* ok       */
                switch (next_input)
                {
                case input::space:  /* space */
                case input::lf: // Linefeed
                case input::cr: // Carriage return
                case input::white:  /* other whitespace */
                    break; // No change
                case input::rcurb:  /* } */
                    pop_object();
                    break;
                case input::rsqrb:  /* ] */
                    pop_array();
                    break;
                case input::c_comma:  /* , */
                    flip_object_key();
                    break;
                case input::slash:  /* / */
                    saved_state_ = state_;
                    state_ = C1;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::invalid_number, json_parser_category()), *this);
                    break;
                }
                break;
            case OB:  /* object   */
                switch (next_input)
                {
                case input::space:  /* space */
                case input::lf: // Linefeed
                case input::cr: // Carriage return
                case input::white:  /* other whitespace */
                    break;
                case input::rcurb:  /* } */
                    pop_key();
                    break;
                case input::quote:  /* " */
                    state_ = ST;
                    break;
                case input::slash:  /* / */
                    saved_state_ = state_;
                    state_ = C1;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::invalid_number, json_parser_category()), *this);
                    break;
                }
                break;
            case KE:  /* key      */
                switch (next_input)
                {
                case input::space:  /* space */
                case input::lf: // Linefeed
                case input::cr: // Carriage return
                case input::white:  /* other whitespace */
                    break;
                case input::quote:  /* " */
                    state_ = ST;
                    break;
                case input::slash:  /* / */
                    saved_state_ = state_;
                    state_ = C1;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::invalid_number, json_parser_category()), *this);
                    break;
                }
                break;
            case CO:  /* colon    */
                switch (next_input)
                {
                case input::space:  /* space */
                case input::lf: // Linefeed
                case input::cr: // Carriage return
                case input::white:  /* other whitespace */
                    break;
                case input::c_colon:  /* : */
                    flip_key_object();
                    state_ = VA;
                    break;
                case input::slash:  /* / */
                    saved_state_ = state_;
                    state_ = C1;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::invalid_number, json_parser_category()), *this);
                    break;
                }
                break;
            case VA:  /* value    */
                switch (next_input)
                {
                case input::space:  /* space */
                case input::lf: // Linefeed
                case input::cr: // Carriage return
                case input::white:  /* other whitespace */
                    break;
                case input::lcurb:  /* {  */
                    push_key(i);
                    break;
                case input::lsqrb:  /* [ */
                    push_array(i);
                    break;
                case input::quote:  /* " */
                    state_ = ST;
                    break;
                case input::slash:  /* / */
                    saved_state_ = state_;
                    state_ = C1;
                    break;
                case input::minus:  /* - */
                    is_negative_ = true;
                    state_ = MI;
                    break;
                case '0': 
                    string_buffer_.push_back(next_char);
                    state_ = ZE;
                    break;
                case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                    string_buffer_.push_back(next_char);
                    state_ = IN;
                    break;
                case input::f:  /* f */
                    state_ = F1;
                    break;
                case input::n:  /* n */
                    state_ = N1;
                    break;
                case input::t:  /* t */
                    state_ = T1;
                    break;
                case input::rcurb:  /* } */
                    err_handler_->error(std::error_code(json_parser_errc::unexpected_end_of_object, json_parser_category()), *this);
                    break;
                case input::rsqrb:  /* ] */
                    err_handler_->error(std::error_code(json_parser_errc::unexpected_end_of_array, json_parser_category()), *this);
                    break;
                case input::c_comma:  /* , */
                    err_handler_->error(std::error_code(json_parser_errc::unexpected_value_separator, json_parser_category()), *this);
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case AR:  /* array    */
                switch (next_input)
                {
                case input::space:  /* space */
                case input::lf: // Linefeed
                case input::cr: // Carriage return
                case input::white:  /* other whitespace */
                    break;
                case input::lcurb:  /* {  */
                    push_key(i);
                    break;
                case input::lsqrb:  /* [ */
                    push_array(i);
                    break;
                case input::rsqrb:  /* ] */
                    pop_array();
                    break;
                case input::quote:  /* " */
                    state_ = ST;
                    break;
                case input::slash:  /* / */
                    saved_state_ = state_;
                    state_ = C1;
                    break;
                case input::minus:  /* - */
                    is_negative_ = true;
                    state_ = MI;
                    break;
                case '0': 
                    string_buffer_.push_back(next_char);
                    state_ = ZE;
                    break;
                case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                    string_buffer_.push_back(next_char);
                    state_ = IN;
                    break;
                case input::f:  /* f */
                    state_ = F1;
                    break;
                case input::n:  /* n */
                    state_ = N1;
                    break;
                case input::t:  /* t */
                    state_ = T1;
                    break;

                case input::rcurb:  /* } */
                    err_handler_->error(std::error_code(json_parser_errc::unexpected_end_of_object, json_parser_category()), *this);
                    break;
                case input::c_colon:  /* : */
                    break;
                case input::c_comma:  /* , */
                    err_handler_->error(std::error_code(json_parser_errc::unexpected_value_separator, json_parser_category()), *this);
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case ST:  /* string   */
                switch (next_input)
                {
                case input::lf: // Linefeed
                case input::cr: // Carriage return
                case input::white:  /* other whitespace */
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                case input::backs:  /* \ */
                    state_ = ES;
                    break;
                case input::quote:  /* " */
                    end_string_value();
					break;
                default:
                    string_buffer_.push_back(next_char);
                    break;
                }
                break;
            case ES:  /* escape   */
                escape_next_char(next_input);
                break;
            case U1:  /* u1       */
                append_codepoint(next_input,next_char);
                state_ = U2;
                break;
            case U2:  /* u1       */
                append_codepoint(next_input,next_char);
                state_ = U3;
                break;
            case U3:  /* u1       */
                append_codepoint(next_input,next_char);
                state_ = U4;
                break;
            case U4:  /* u1       */
                append_codepoint(next_input,next_char);
                if (cp_ >= min_lead_surrogate && cp_ <= max_lead_surrogate)
                {
                    state_ = SP;
                }
                else
                {
                    json_char_traits<Char, sizeof(Char)>::append_codepoint_to_string(cp_, string_buffer_);
                    state_ = ST;
                }
                break;
            case SP:  /* SP       */
                switch (next_input)
                {
                case input::backs:  /* \ */
                    cp2_ = 0;
                    state_ = U5;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case U5:  /* u5       */
                switch (next_input)
                {
                case input::u:  /* u */
                    state_ = U6;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case U6:  /* u1       */
                append_second_codepoint(next_input,next_char);
                state_ = U7;
                break;
            case U7:  /* u1       */
                append_second_codepoint(next_input,next_char);
                state_ = U8;
                break;
            case U8:  /* u1       */
                append_second_codepoint(next_input,next_char);
                state_ = U9;
                break;
            case U9:  /* u1       */
                append_second_codepoint(next_input,next_char);
                json_char_traits<Char, sizeof(Char)>::append_codepoint_to_string(cp_, string_buffer_);
                state_ = ST;
                break;
            case MI:  /* minus    */
                switch (next_input)
                {
                case '0': 
                    string_buffer_.push_back(next_char);
                    state_ = ZE;
                    break;
                case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                    string_buffer_.push_back(next_char);
                    state_ = IN;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case ZE:  /* zero     */
                switch (next_input)
                {
                case input::space:  /* space */
                case input::lf: // Linefeed
                case input::cr: // Carriage return
                case input::white:  /* other whitespace */
                    end_integer_value();
                    state_ = OK;
                    break; // No change
                case input::rcurb:  /* } */
                    end_integer_value();
                    pop_object();
                    break;
                case input::rsqrb:  /* ] */
                    end_integer_value();
                    pop_array();
                    break;
                case input::point:  /* . */
                    string_buffer_.push_back(next_char);
                    state_ = FR;
                    break;
                case input::c_comma:  /* , */
                    end_integer_value();
                    flip_object_key();
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case IN:  /* integer  */
                switch (next_input)
                {
                case input::space:  /* space */
                case input::lf: // Linefeed
                case input::cr: // Carriage return
                case input::white:  /* other whitespace */
                    end_integer_value();
                    state_ = OK;
                    break; // No change
                case input::rcurb:  /* } */
                    end_integer_value();
                    pop_object();
                    break;
                case input::rsqrb:  /* ] */
                    end_integer_value();
                    pop_array();
                    break;
                case '0': 
                case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                    string_buffer_.push_back(next_char);
                    state_ = IN;
                    break;
                case input::point:  /* . */
                    string_buffer_.push_back(next_char);
                    state_ = FR;
                    break;
                case input::c_comma:  /* , */
                    end_integer_value();
                    flip_object_key();
                    break;
                case input::e:  /* e */
                    string_buffer_.push_back(next_char);
                    state_ = E1;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case FR:  /* fraction */
                switch (next_input)
                {
                case input::space:  /* space */
                case input::lf: // Linefeed
                case input::cr: // Carriage return
                case input::white:  /* other whitespace */
                    end_frac_value();
                    state_ = OK;
                    break; // No change
                case input::rcurb:  /* } */
                    end_frac_value();
                    pop_object();
                    break;
                case input::rsqrb:  /* ] */
                    end_frac_value();
                    pop_array();
                    break;
                case '0': 
                case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                    string_buffer_.push_back(next_char);
                    state_ = FR;
                    break;
                case input::c_comma:  /* , */
                    end_frac_value();
                    flip_object_key();
                    break;
                case input::e:  /* e */
                    string_buffer_.push_back(next_char);
                    state_ = E1;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case E1:  /* e        */
                switch (next_input)
                {
                case input::plus:   /* + */
                    state_ = E2;
                    break;
                case input::minus:  /* - */
                    string_buffer_.push_back(next_char);
                    state_ = E2;
                    break;
                case '0': 
                case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                    string_buffer_.push_back(next_char);
                    state_ = E3;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case E2:  /* ex       */
                switch (next_input)
                {
                case '0': 
                case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                    string_buffer_.push_back(next_char);
                    state_ = E3;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case E3:  /* exp      */
                switch (next_input)
                {
                case input::space:  /* space */
                case input::lf: // Linefeed
                case input::cr: // Carriage return
                case input::white:  /* other whitespace */
                    end_frac_value();
                    state_ = OK;
                    break; // No change
                case input::rcurb:  /* } */
                    end_frac_value();
                    pop_object();
                    break;
                case input::rsqrb:  /* ] */
                    end_frac_value();
                    pop_array();
                    break;
                case input::c_comma:  /* , */
                    end_frac_value();
                    flip_object_key();
                    break;
                case '0': 
                case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                    string_buffer_.push_back(next_char);
                    state_ = E3;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case T1:  /* tr       */
                switch (next_input)
                {
                case input::r:
                    state_ = T2;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case T2:  /* tru      */
                switch (next_input)
                {
                case input::u:  /* u */
                    state_ = T3;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case T3:  /* true     */
                switch (next_input)
                {
                case input::e: 
                    handler_->value(true, *this);
                    state_ = OK;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case F1:  /* fa       */
                switch (next_input)
                {
                case input::a:  /* u */
                    state_ = F2;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case F2:  /* fal      */
                switch (next_input)
                {
                case input::l: 
                    state_ = F3;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case F3:  /* fals     */
                switch (next_input)
                {
                case input::s:  /* u */
                    state_ = F4;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case F4:  /* false    */
                switch (next_input)
                {
                case input::e:  /* u */
                    handler_->value(false, *this);
                    state_ = OK;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case N1:  // nu 
                switch (next_input)
                {
                case input::u:  /* u */
                    state_ = N2;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case N2:  /* nul      */
                switch (next_input)
                {
                case input::l:  /* l */
                    state_ = N3;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case N3:  /* null     */
                switch (next_input)
                {
                case input::l:  /* l */
                    handler_->value(null_type(), *this);
                    state_ = OK;
                    break;
                default:
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    break;
                }
                break;
            case C1:  // /
                switch (next_input)
                {
                case input::star:   /* * */
                    state_ = C2;
                    break;
                default:    
                    err_handler_->error(std::error_code(json_parser_errc::expected_container, json_parser_category()), *this);
                    break;
                }
                break;
            case C2:  // /*
                switch (next_input)
                {
                case input::star:   /* * */
                    state_ = C3;
                    break;
                }
                break;
            case C3:  // *
                switch (next_input)
                {
                case input::slash:  /* slash */
                    state_ = saved_state_;
                    break;
                default:    /* everything else */
                    state_ = C2;
                    break;
                }
                break;
            }
        }

        return true;
    }

    int count_members(size_t start_index)
    {
        int start_top = top_;
        int start_state = state_;
        stack_[top_].minimum_structure_capacity = 0;
        int saved_state;

        bool done = false;
        for (size_t i = start_index; !done && i < buffer_length_; ++i)
        {
            int next_char = buffer_[i];

            /*
                After calling new_JSON_checker, call this function for each character (or
                partial character) in your JSON text. It can accept UTF-8, UTF-16, or
                UTF-32. It returns true if things are looking ok so far. If it rejects the
                text, it deletes the json_parser object and returns false.
            */
            int next_input, next_state;
            /*
                Determine the character's class.
            */
            if (next_char < 0)
            {
                top_ = start_top;
                state_ = start_state;
                return false;
            }
            if (next_char >= 128)
            {
                next_input = input::etc;
            }
            else
            {
                next_input = ascii_class[next_char];
                if (next_input <= __)
                {
                    top_ = start_top;
                    state_ = start_state;
                    return false;
                }
            }
            /*
                Get the next state from the state transition table.
            */
            next_state = state_transition_table[state_][next_input];
            switch (state_)
            {
            case ZE:
            case IN:
            case FR:
            case E1:
            case E2:
            case E3:
                switch (next_state)
                {
                case ZE:
                case IN:
                case FR:
                case E1:
                case E2:
                case E3:
                    break;
                default:
                    if (top_ == start_top)
                    {
                        ++stack_[start_top].minimum_structure_capacity;
                    }
                }
                break;
            }

            switch (next_state)
            {
            case C1:
                saved_state = state_;
                state_ = next_state;
                break;
            case C4:
                state_ = saved_state;
                break;
            case T4:
            case F5:
            case N4:
                if (top_ == start_top)
                {
                    ++stack_[start_top].minimum_structure_capacity;
                }
                state_ = OK;
                break;
            case S2:
            case S3:
            case S4:
            case S5:
            case Sb:
            case Sf:
            case Sn:
            case Sr:
            case St:
            case U4:
            case U10:
                state_ = ST;
                break;
            case __:
                top_ = start_top;
                state_ = start_state;
                return false;
                break;
            case -9:
                if (top_ == start_top)
                {
                    done = true;
                }
                else
                {
                    if (!pop(mode::key))
                    {
                        top_ = start_top;
                        state_ = start_state;
                        return false;
                    }
                    if (top_ == start_top)
                    {
                        ++stack_[start_top].minimum_structure_capacity;
                    }
                    state_ = OK;
                }
                break;

            /* } */ case -8:
                if (top_ == start_top)
                {
                    done = true;
                }
                else
                {
                    if (!pop(mode::object))
                    {
                        top_ = start_top;
                        state_ = start_state;
                        return false;
                    }
                    if (top_ == start_top)
                    {
                        ++stack_[start_top].minimum_structure_capacity;
                    }
                    state_ = OK;
                }
                break;

            /* ] */ case -7:
                if (top_ == start_top)
                {
                    done = true;
                }
                else
                {
                    if (!pop(mode::array))
                    {
                        top_ = start_top;
                        state_ = start_state;
                        return false;
                    }
                    if (top_ == start_top)
                    {
                        ++stack_[start_top].minimum_structure_capacity;
                    }
                    state_ = OK;
                }
                break;

            /* { */ case -6:
                if (!push(mode::key))
                {
                    top_ = start_top;
                    state_ = start_state;
                    return false;
                }
                state_ = OB;
                break;

            /* [ */ case -5:
                if (!push(mode::array))
                {
                    top_ = start_top;
                    state_ = start_state;
                    return false;
                }
                state_ = AR;
                break;

            /* " */ case -4:
                switch (stack_[top_].mode)
                {
                case mode::key:
                    state_ = CO;
                    break;
                case mode::array:
                case mode::object:
                    state_ = OK;
                    ++stack_[start_top].minimum_structure_capacity;
                    break;
                default:
                    top_ = start_top;
                    state_ = start_state;
                    return false;
                }
                break;

            /* , */ case -3:
                switch (stack_[top_].mode)
                {
                case mode::object:
                    // A comma causes a flip from object mode to key mode.
                    if (!flip(mode::object, mode::key))
                    {
                        top_ = start_top;
                        state_ = start_state;
                        return false;
                    }
                    state_ = KE;
                    break;
                case mode::array:
                    state_ = VA;
                    break;
                default:
                    top_ = start_top;
                    state_ = start_state;
                    return false;
                }
                break;

            /* : */ case -2:
                // A colon causes a flip from key mode to object mode.
                if (!flip(mode::key, mode::object))
                {
                    top_ = start_top;
                    state_ = start_state;
                    return false;
                }
                state_ = VA;
                break;
                /*
                    Bad action.
                */
            default:
                state_ = next_state;
                break;
            }

        }
        state_ = start_state;
        top_ = start_top;

        return true;
    }

    bool check_done()
    {
        /*
            The JSON_checker_done function should be called after all of the characters
            have been processed, but only if every call to JSON_checker_char returned
            true. This function returns true if the JSON text was accepted.
        */
        bool result = (state_ == OK) && pop(mode::done);
        return result;
    }

    int state_;
    int depth_;
    int top_;
    std::vector<stack_item> stack_;
    basic_json_input_handler<Char> *handler_;
    basic_parse_error_handler<Char> *err_handler_;
    std::basic_istream<Char> *is_;
    unsigned long column_;
    unsigned long line_;
    Char c_;
    bool eof_;
    uint32_t cp_;
    uint32_t cp2_;
    std::vector<Char> buffer_;
    size_t buffer_length_;
    size_t buffer_capacity_;
    std::basic_string<Char> string_buffer_;
    bool is_negative_;
    int saved_state_;
};

template<typename Char>
int basic_json_reader<Char>::ascii_class[128] =
{
    /*
        This array maps the 128 ASCII characters into character classes.
        The remaining Unicode characters should be mapped to input::etc.
        Non-whitespace control characters are errors.
    */
    __,      __,      __,      __,      __,      __,      __,      __,
    __,      input::white, input::lf, __,      __,      input::cr, __,      __,
    __,      __,      __,      __,      __,      __,      __,      __,
    __,      __,      __,      __,      __,      __,      __,      __,

    input::space, input::etc,   input::quote, input::etc,   input::etc,   input::etc,   input::etc,   input::etc,
    input::etc,   input::etc,   input::star,   input::plus,  input::c_comma, input::minus, input::point, input::slash,
    '0',  '1', '2', '3', '4', '5', '6', '7',
    '8', '9', input::c_colon, input::etc,   input::etc,   input::etc,   input::etc,   input::etc,

    input::etc,   input::ABCDF, input::ABCDF, input::ABCDF, input::ABCDF, input::E,     input::ABCDF, input::etc,
    input::etc,   input::etc,   input::etc,   input::etc,   input::etc,   input::etc,   input::etc,   input::etc,
    input::etc,   input::etc,   input::etc,   input::etc,   input::etc,   input::etc,   input::etc,   input::etc,
    input::etc,   input::etc,   input::etc,   input::lsqrb, input::backs, input::rsqrb, input::etc,   input::etc,

    input::etc,   input::a, input::b, input::c, input::d, input::e, input::f, input::etc,
    input::etc,   input::etc,   input::etc,   input::etc,   input::l, input::etc,   input::n, input::etc,
    input::etc,   input::etc,   input::r, input::s, input::t, input::u, input::etc,   input::etc,
    input::etc,   input::etc,   input::etc,   input::lcurb, input::etc,   input::rcurb, input::etc,   input::etc
};

template<typename Char>
int basic_json_reader<Char>::state_transition_table[state_count][input::input_count] = {
    /*
        The state transition table takes the current state and the current symbol,
        and returns either a new state or an action. An action is represented as a
        negative number. A JSON text is accepted if at the end of the text the
        state is OK and if the mode is MODE_DONE.

                 white                                      1-9                                                                  ABCDF    etc
                 space   {  }  [  ]  :  ,  "  \  /  *  +  -  .  0  |  a  b  c  d  e  f  l  n  r  s  t  u  |  E  |*/
    /*start  GO*/{GO,GO,GO,GO,-6,__,-5,__,__,__,__,__,C1,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
    /*ok     OK*/{OK,OK,OK,OK,__,-8,__,-7,__,-3,__,__,C1,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
    /*object OB*/{OB,OB,OB,OB,__,-9,__,__,__,__,ST,__,C1,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
    /*key    KE*/{KE,KE,KE,KE,__,__,__,__,__,__,ST,__,C1,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
    /*colon  CO*/{CO,CO,CO,CO,__,__,__,__,-2,__,__,__,C1,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
    /*value  VA*/{VA,VA,VA,VA,-6,__,-5,__,__,__,ST,__,C1,__,__,MI,__,ZE,IN,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__},
    /*array  AR*/{AR,AR,AR,AR,-6,__,-5,-7,__,__,ST,__,C1,__,__,MI,__,ZE,IN,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__},
    /*string ST*/{S2,__,__,__,S2,S2,S2,S2,S2,S2,-4,ES,S2,S2,S2,S2,S2,S2,S2,S2,S2,S2,S2,S2,S2,S2,S2,S2,S2,S2,S2,S2,S2,S2},
    /*escape ES*/{__,__,__,__,__,__,__,__,__,__,S3,S4,S5,__,__,__,__,__,__,__,Sb,__,__,__,Sf,__,Sn,Sr,__,St,U0,__,__,__},
    /*u1     U0*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,U1,U1,U1,U1,U1,U1,U1,U1,__,__,__,__,__,__,U1,U1,__},
    /*u2     U1*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,U2,U2,U2,U2,U2,U2,U2,U2,__,__,__,__,__,__,U2,U2,__},
    /*u3     U2*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,U3,U3,U3,U3,U3,U3,U3,U3,__,__,__,__,__,__,U3,U3,__},
    /*u4     U3*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,U4,U4,U4,U4,U4,U4,U4,U4,__,__,__,__,__,__,U4,U4,__},
    /*SP     SP*/{__,__,__,__,__,__,__,__,__,__,__,U5,C1,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
    /*escape U5*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,U6,__,__,__},
    /*u1     U6*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,U7,U7,U7,U7,U7,U7,U7,U7,__,__,__,__,__,__,U7,U7,__},
    /*u2     U7*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,U8,U8,U8,U8,U8,U8,U8,U8,__,__,__,__,__,__,U8,U8,__},
    /*u3     U8*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,U9,U9,U9,U9,U9,U9,U9,U9,__,__,__,__,__,__,U9,U9,__},
    /*u4     U9*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,U10,U10,U10,U10,U10,U10,U10,U10,__,__,__,__,__,__,U10,U10,__},
    /*minus  MI*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,ZE,IN,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
    /*zero   ZE*/{OK,OK,OK,OK,__,-8,__,-7,__,-3,__,__,C1,__,__,__,FR,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
    /*int    IN*/{OK,OK,OK,OK,__,-8,__,-7,__,-3,__,__,C1,__,__,__,FR,IN,IN,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__},
    /*frac   FR*/{OK,OK,OK,OK,__,-8,__,-7,__,-3,__,__,C1,__,__,__,__,FR,FR,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__},
    /*e      E1*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,E2,E2,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
    /*ex     E2*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
    /*exp    E3*/{OK,OK,OK,OK,__,-8,__,-7,__,-3,__,__,C1,__,__,__,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
    /*tr     T1*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,__,__,__,__,__,__,__,__,__,__,T2,__,__,__,__,__,__},
    /*tru    T2*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,T3,__,__,__},
    /*true   T3*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,__,__,__,__,__,__,T4,__,__,__,__,__,__,__,__,__,__},
    /*fa     F1*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,__,__,F2,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
    /*fal    F2*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,__,__,__,__,__,__,__,__,F3,__,__,__,__,__,__,__,__},
    /*fals   F3*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F4,__,__,__,__,__},
    /*false  F4*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,__,__,__,__,__,__,F5,__,__,__,__,__,__,__,__,__,__},
    /*nu     N1*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,N2,__,__,__},
    /*nul    N2*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,__,__,__,__,__,__,__,__,N3,__,__,__,__,__,__,__,__},
    /*null   N3*/{__,__,__,__,__,__,__,__,__,__,__,__,C1,__,__,__,__,__,__,__,__,__,__,__,__,N4,__,__,__,__,__,__,__,__},
    /*       C1*/{__,__,__,__,__,__,__,__,__,__,__,__,__,C2,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
    /*       C2*/{C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C3,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2},
    /*       C3*/{C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C4,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2},
};

typedef basic_json_reader<char> json_reader;
typedef basic_json_reader<wchar_t> wjson_reader;

}

#endif

