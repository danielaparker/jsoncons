#ifndef JSONCONS_JSONPARSER_HPP
#define JSONCONS_JSONPARSER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <istream>

namespace jsoncons {

class json_parser_exception : public std::exception
{
public:
    json_parser_exception(std::string s, unsigned long line_number)
    {
        std::ostringstream os(s);
        os << " on line " << line_number;
        message_ = os.str();
    }
    const char* what() const
    {
        return message_.c_str();
    }
private:
    std::string message_;
};

#define JSONCONS_THROW_PARSER_EXCEPTION(x,n) throw json_parser_exception(x,n)

class json_variant;
class json_object;

class json_parser
{
public:
    json_object* parse(std::istream& is);
    json_object* parse_object(std::istream& is);
    json_variant* parse_separator_value(std::istream& is);
    json_variant* parse_value(std::istream& is);
    json_variant* parse_number(std::istream& is, char c);
    json_variant* parse_array(std::istream& is);
    void parse_string(std::istream& is);
    void ignore_till_end_of_line(std::istream& is);
    bool read_until_match_fails(std::istream& is, const char *s);
    unsigned int decode_unicode_codepoint(std::istream& is);
    unsigned int decode_unicode_escape_sequence(std::istream& is);
private:
    unsigned long line_number_;
    std::string buffer_;
};

}

#endif
