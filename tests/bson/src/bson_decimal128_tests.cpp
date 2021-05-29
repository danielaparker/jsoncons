/*
 * Copyright 2015 MongoDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <jsoncons/json.hpp>
#include <jsoncons_ext/bson/bson.hpp>
#include <jsoncons_ext/bson/bson_decimal128.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>
#include <array>
#include <catch/catch.hpp>

using namespace jsoncons;
using namespace jsoncons::bson;

TEST_CASE("test_decimal128_to_string__infinity")
{
   char bid_string[decimal128_limits::recommended_buffer_size+1];

   bson_decimal128_t positive_infinity(0x7800000000000000, 0);
   bson_decimal128_t negative_infinity(0xf800000000000000, 0);

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), positive_infinity);
   CHECK (!strcmp (bid_string, "Infinity"));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), negative_infinity);
   CHECK (!strcmp (bid_string, "-Infinity"));
}


TEST_CASE("test_decimal128_to_string__nan")
{
   /* All the above should just be NaN. */
   char bid_string[decimal128_limits::recommended_buffer_size+1];

   bson_decimal128_t dec_pnan(0x7c00000000000000, 0);
   bson_decimal128_t dec_nnan(0xfc00000000000000, 0);
   bson_decimal128_t dec_psnan(0x7e00000000000000, 0);
   bson_decimal128_t dec_nsnan(0xfe00000000000000, 0);
   bson_decimal128_t dec_payload_nan(0x7e00000000000000, 12);

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), dec_pnan);
   CHECK (!strcmp (bid_string, "NaN"));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), dec_nnan);
   CHECK (!strcmp (bid_string, "NaN"));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), dec_psnan);
   CHECK (!strcmp (bid_string, "NaN"));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), dec_nsnan);
   CHECK (!strcmp (bid_string, "NaN"));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), dec_payload_nan);
   CHECK (!strcmp (bid_string, "NaN"));
}


TEST_CASE("test_decimal128_to_string__regular")
{
   char bid_string[decimal128_limits::recommended_buffer_size+1];

   bson_decimal128_t one(0x3040000000000000, 0x0000000000000001);
   bson_decimal128_t zero(0x3040000000000000, 0x0000000000000000);
   bson_decimal128_t two(0x3040000000000000, 0x0000000000000002);
   bson_decimal128_t negative_one(0xb040000000000000, 0x0000000000000001);
   bson_decimal128_t negative_zero(0xb040000000000000, 0x0000000000000000);
   bson_decimal128_t tenth(0x303e000000000000, 0x0000000000000001); /* 0.1 */
   /* 0.001234 */
   bson_decimal128_t smallest_regular(0x3034000000000000, 0x00000000000004d2);
   /* 12345789012 */
   bson_decimal128_t largest_regular(0x3040000000000000, 0x0000001cbe991a14);
   /* 0.00123400000 */
   bson_decimal128_t trailing_zeros(0x302a000000000000, 0x00000000075aef40);
   /* 0.1234567890123456789012345678901234 */
   bson_decimal128_t all_digits(0x2ffc3cde6fff9732, 0xde825cd07e96aff2);

   /* 5192296858534827628530496329220095 */
   bson_decimal128_t full_house(0x3040ffffffffffff, 0xffffffffffffffff);

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), one);
   CHECK (!strcmp ("1", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), zero);
   CHECK (!strcmp ("0", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), two);
   CHECK (!strcmp ("2", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), negative_one);
   CHECK (!strcmp ("-1", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), negative_zero);
   CHECK (!strcmp ("-0", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), tenth);
   CHECK (!strcmp ("0.1", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), smallest_regular);
   CHECK (!strcmp ("0.001234", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), largest_regular);
   CHECK (!strcmp ("123456789012", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), trailing_zeros);
   CHECK (!strcmp ("0.00123400000", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), all_digits);
   CHECK (!strcmp ("0.1234567890123456789012345678901234", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), full_house);
   CHECK (!strcmp ("5192296858534827628530496329220095", bid_string));
}


TEST_CASE("test_decimal128_to_string__scientific")
{
   char bid_string[decimal128_limits::recommended_buffer_size+1];

   bson_decimal128_t huge(0x5ffe314dc6448d93, 0x38c15b0a00000000); /* 1.000000000000000000000000000000000E+6144 */
   bson_decimal128_t tiny(0x0000000000000000, 0x0000000000000001); /* 1E-6176 */
   bson_decimal128_t neg_tiny(0x8000000000000000, 0x0000000000000001); /* -1E-6176 */
   bson_decimal128_t large(0x3108000000000000, 0x000009184db63eb1); /* 9.999987654321E+112 */
   bson_decimal128_t largest(0x5fffed09bead87c0, 0x378d8e63ffffffff); /* 9.999999999999999999999999999999999E+6144 */
   bson_decimal128_t tiniest(0x0001ed09bead87c0, 0x378d8e63ffffffff); /* 9.999999999999999999999999999999999E-6143 */
   bson_decimal128_t trailing_zero(0x304c000000000000, 0x000000000000041a); /* 1.050E9 */
   bson_decimal128_t one_trailing_zero(0x3042000000000000, 0x000000000000041a); /* 1.050E4 */
   bson_decimal128_t move_decimal(0x3040000000000000, 0x0000000000000069); /* 105 */
   bson_decimal128_t move_decimal_after(0x3042000000000000, 0x0000000000000069); /* 1.05E3 */
   bson_decimal128_t trailing_zero_no_decimal(0x3046000000000000, 0x0000000000000001); /* 1E3 */

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), huge);
   CHECK (
      !strcmp ("1.000000000000000000000000000000000E+6144", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), tiny);
   CHECK (!strcmp ("1E-6176", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), neg_tiny);
   CHECK (!strcmp ("-1E-6176", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), neg_tiny);
   CHECK (!strcmp ("-1E-6176", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), large);
   CHECK (!strcmp ("9.999987654321E+112", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), largest);
   CHECK (
      !strcmp ("9.999999999999999999999999999999999E+6144", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), tiniest);
   CHECK (
      !strcmp ("9.999999999999999999999999999999999E-6143", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), trailing_zero);
   CHECK (!strcmp ("1.050E+9", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), one_trailing_zero);
   CHECK (!strcmp ("1.050E+4", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), move_decimal);
   CHECK (!strcmp ("105", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), move_decimal_after);
   CHECK (!strcmp ("1.05E+3", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), trailing_zero_no_decimal);
   CHECK (!strcmp ("1E+3", bid_string));
}


TEST_CASE("test_decimal128_to_string__zeros")
{
   char bid_string[decimal128_limits::recommended_buffer_size+1];

   bson_decimal128_t zero(0x3040000000000000, 0x0000000000000000); /* 0 */
   bson_decimal128_t pos_exp_zero(0x3298000000000000, 0x0000000000000000); /* 0E+300 */
   bson_decimal128_t neg_exp_zero(0x2b90000000000000, 0x0000000000000000); /* 0E-600 */

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), zero);
   CHECK (!strcmp ("0", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), pos_exp_zero);
   CHECK (!strcmp ("0E+300", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), neg_exp_zero);
   CHECK (!strcmp ("0E-600", bid_string));
}


#define IS_NAN(dec) (dec).high == 0x7c00000000000000ull


TEST_CASE("test_decimal128_from_string__invalid_inputs")
{
   bson_decimal128_t dec;

   {
       char buf[] = ".";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = ".e";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = ""; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "invalid"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "in"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "i"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "E02"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "..1"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "1abcede"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "1.24abc"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "1.24abcE+02"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "1.24E+02abc2d"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "E+02"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "e+02"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = ".";
       decimal128_from_chars(buf, buf+1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = ".e";
       decimal128_from_chars(buf, buf+2, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "";
       decimal128_from_chars(buf, buf, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "invalid";
       decimal128_from_chars(buf, buf+7, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "in";
       decimal128_from_chars(buf, buf+2, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "i";
       decimal128_from_chars(buf, buf+1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "E02";
       decimal128_from_chars(buf, buf+3, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "..1";
       decimal128_from_chars(buf, buf+3, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "1abcede";
       decimal128_from_chars(buf, buf+7, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "1.24abc";
       decimal128_from_chars(buf, buf+7, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "1.24abcE+02";
       decimal128_from_chars(buf, buf+11, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "1.24E+02abc2d";
       decimal128_from_chars(buf, buf+13, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "E+02";
       decimal128_from_chars(buf, buf+4, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "e+02";
       decimal128_from_chars(buf, buf+4, dec);
       CHECK (IS_NAN (dec));
   }
}

TEST_CASE("test_decimal128_from_string__nan")
{
   bson_decimal128_t dec;

   {
       char buf[] = "NaN"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "+NaN"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "-NaN"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "-nan"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "1e"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "+nan"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "nan"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "Nan"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "+Nan"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "-Nan"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "NaN"; 
       decimal128_from_chars(buf, buf+3, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "+NaN";
       decimal128_from_chars(buf, buf+4, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "-NaN";
       decimal128_from_chars(buf, buf+4, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "-nan";
       decimal128_from_chars(buf, buf+4, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "1e";
       decimal128_from_chars(buf, buf+2, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "+nan";
       decimal128_from_chars(buf, buf+4, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "nan";
       decimal128_from_chars(buf, buf+3, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "Nan";
       decimal128_from_chars(buf, buf+3, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "+Nan";
       decimal128_from_chars(buf, buf+4, dec);
       CHECK (IS_NAN (dec));
   }
   {
       char buf[] = "-Nan";
       decimal128_from_chars(buf, buf+4, dec);
       CHECK (IS_NAN (dec));
   }
}


#define IS_PINFINITY(dec) (dec).high == 0x7800000000000000
#define IS_NINFINITY(dec) (dec).high == 0xf800000000000000


TEST_CASE("test_decimal128_from_string__infinity")
{
   bson_decimal128_t dec;

   {
       char buf[] = "Infinity"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_PINFINITY (dec));
   }
   {
       char buf[] = "+Infinity"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_PINFINITY (dec));
   }
   {
       char buf[] = "+Inf"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_PINFINITY (dec));
   }
   {
       char buf[] = "-Inf"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NINFINITY (dec));
   }
   {
       char buf[] = "-Infinity"; 
       decimal128_from_chars(buf, buf+sizeof(buf)-1, dec);
       CHECK (IS_NINFINITY (dec));
   }

   {
       char buf[] = "Infinity";
       decimal128_from_chars(buf, buf+8, dec);
       CHECK (IS_PINFINITY (dec));
   }
   {
       char buf[] = "+Infinity";
       decimal128_from_chars(buf, buf+9, dec);
       CHECK (IS_PINFINITY (dec));
   }
   {
       char buf[] = "+Inf";
       decimal128_from_chars(buf, buf+4, dec);
       CHECK (IS_PINFINITY (dec));
   }
   {
       char buf[] = "-Inf";
       decimal128_from_chars(buf, buf+4, dec);
       CHECK (IS_NINFINITY (dec));
   }
   {
       char buf[] = "-Infinity";
       decimal128_from_chars(buf, buf+9, dec);
       CHECK (IS_NINFINITY (dec));
   }
}

TEST_CASE("test_decimal128_from_string__simple")
{
   bson_decimal128_t one;
   bson_decimal128_t negative_one;
   bson_decimal128_t zero;
   bson_decimal128_t negative_zero;
   bson_decimal128_t number;
   bson_decimal128_t number_two;
   bson_decimal128_t negative_number;
   bson_decimal128_t fractional_number;
   bson_decimal128_t leading_zeros;
   bson_decimal128_t leading_insignificant_zeros;

   {
       char buf[] = "1";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, one);
   }
   {
       char buf[] = "-1";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, negative_one);
   }
   {
       char buf[] = "0";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, zero);
   }
   {
       char buf[] = "-0";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, negative_zero);
   }
   {
       char buf[] = "12345678901234567";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, number);
   }
   {
       char buf[] = "989898983458";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, number_two);
   }
   {
       char buf[] = "-12345678901234567";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, negative_number);
   }
   {
       char buf[] = "0.12345";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, fractional_number);
   }
   {
       char buf[] = "0.0012345";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, leading_zeros);
   }
   {
       char buf[] = "00012345678901234567";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, leading_insignificant_zeros);
   }

   CHECK (one == bson_decimal128_t(0x3040000000000000, 0x0000000000000001));
   CHECK (negative_one == bson_decimal128_t(0xb040000000000000, 0x0000000000000001));
   CHECK (zero == bson_decimal128_t(0x3040000000000000, 0x0000000000000000));
   CHECK (negative_zero == bson_decimal128_t(0xb040000000000000, 0x0000000000000000));
   CHECK (number == bson_decimal128_t(0x3040000000000000, 0x002bdc545d6b4b87));
   CHECK (number_two == bson_decimal128_t(0x3040000000000000, 0x000000e67a93c822));
   CHECK (negative_number == bson_decimal128_t(0xb040000000000000, 0x002bdc545d6b4b87));
   CHECK (fractional_number == bson_decimal128_t(0x3036000000000000, 0x0000000000003039));
   CHECK (leading_zeros == bson_decimal128_t(0x3032000000000000, 0x0000000000003039));
   CHECK (leading_insignificant_zeros == bson_decimal128_t(0x3040000000000000, 0x002bdc545d6b4b87));

   {
       char buf[] = "1";
       decimal128_from_chars(buf, buf+1, one);
   }
   {
       char buf[] = "-1";
       decimal128_from_chars(buf, buf+2, negative_one);
   }
   {
       char buf[] = "0";
       decimal128_from_chars(buf, buf+1, zero);
   }
   {
       char buf[] = "-0";
       decimal128_from_chars(buf, buf+2, negative_zero);
   }
   {
       char buf[] = "12345678901234567";
       decimal128_from_chars(buf, buf+17, number);
   }
   {
       char buf[] = "989898983458";
       decimal128_from_chars(buf, buf+12, number_two);
       CHECK (number_two == bson_decimal128_t(0x3040000000000000, 0x000000e67a93c822));
   }
   {
       char buf[] = "-12345678901234567";
       decimal128_from_chars(buf, buf+18, negative_number);
   }

   {
       char buf[] = "0.12345";
       decimal128_from_chars(buf, buf+7, fractional_number);
   }
   {
       char buf[] = "0.0012345";
       decimal128_from_chars(buf, buf+9, leading_zeros);
   }

   {
       char buf[] = "00012345678901234567";
       decimal128_from_chars(buf, buf+sizeof(buf)+20, leading_insignificant_zeros);
   }

   CHECK (one == bson_decimal128_t(0x3040000000000000, 0x0000000000000001));
   CHECK (negative_one == bson_decimal128_t(0xb040000000000000, 0x0000000000000001));
   CHECK (zero == bson_decimal128_t(0x3040000000000000, 0x0000000000000000));
   CHECK (negative_zero == bson_decimal128_t(0xb040000000000000, 0x0000000000000000));
   CHECK (number == bson_decimal128_t(0x3040000000000000, 0x002bdc545d6b4b87));
   CHECK (negative_number == bson_decimal128_t(0xb040000000000000, 0x002bdc545d6b4b87));
   CHECK (fractional_number == bson_decimal128_t(0x3036000000000000, 0x0000000000003039));
   CHECK (leading_zeros == bson_decimal128_t(0x3032000000000000, 0x0000000000003039));
   CHECK (leading_insignificant_zeros == bson_decimal128_t(0x3040000000000000, 0x002bdc545d6b4b87));
}


TEST_CASE("test_decimal128_from_string__scientific")
{
   bson_decimal128_t ten;
   bson_decimal128_t ten_again;
   bson_decimal128_t one;
   bson_decimal128_t huge_exp;
   bson_decimal128_t tiny_exp;
   bson_decimal128_t fractional;
   bson_decimal128_t trailing_zeros;

   {
       char buf[] = "10e0";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, ten);
       CHECK (ten == bson_decimal128_t(0x3040000000000000, 0x000000000000000a));
   }
   {
       char buf[] = "1e1";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, ten_again);
   }
   {
       char buf[] = "10e-1";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, one);
   }

   CHECK (ten == bson_decimal128_t(0x3040000000000000, 0x000000000000000a));
   CHECK (ten_again == bson_decimal128_t(0x3042000000000000, 0x0000000000000001));
   CHECK (one == bson_decimal128_t(0x303e000000000000, 0x000000000000000a));

   {
       char buf[] = "12345678901234567e6111";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, huge_exp);
   }
   {
       char buf[] = "1e-6176";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, tiny_exp);
   }

   CHECK (huge_exp == bson_decimal128_t(0x5ffe000000000000, 0x002bdc545d6b4b87));
   CHECK (tiny_exp == bson_decimal128_t(0x0000000000000000, 0x0000000000000001));

   {
       char buf[] = "-100E-10";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, fractional);
   }
   {
       char buf[] = "10.50E8";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, trailing_zeros);
   }

   CHECK (fractional == bson_decimal128_t(0xb02c000000000000, 0x0000000000000064));
   CHECK (trailing_zeros == bson_decimal128_t(0x304c000000000000, 0x000000000000041a));


   {
       char buf[] = "10e0";
       decimal128_from_chars(buf, buf+4, ten);
   }
   {
       char buf[] = "1e1";
       decimal128_from_chars(buf, buf+3, ten_again);
   }
   {
       char buf[] = "10e-1";
       decimal128_from_chars(buf, buf+5, one);
   }

   CHECK (ten_again == bson_decimal128_t(0x3042000000000000, 0x0000000000000001));
   CHECK (one == bson_decimal128_t(0x303e000000000000, 0x000000000000000a));

   {
       char buf[] = "12345678901234567e6111";
       decimal128_from_chars(buf, buf+22, huge_exp);
   }
   {
       char buf[] = "1e-6176";
       decimal128_from_chars(buf, buf+7, tiny_exp);
   }

   CHECK (huge_exp == bson_decimal128_t(0x5ffe000000000000, 0x002bdc545d6b4b87));
   CHECK (tiny_exp == bson_decimal128_t(0x0000000000000000, 0x0000000000000001));

   {
       char buf[] = "-100E-10";
       decimal128_from_chars(buf, buf+8, fractional);
   }
   {
       char buf[] = "10.50E8";
       decimal128_from_chars(buf, buf+7, trailing_zeros);
   }

   CHECK (fractional == bson_decimal128_t(0xb02c000000000000, 0x0000000000000064));
   CHECK (trailing_zeros == bson_decimal128_t(0x304c000000000000, 0x000000000000041a));
}


TEST_CASE("test_decimal128_from_string__large")
{
   bson_decimal128_t large;
   bson_decimal128_t all_digits;
   bson_decimal128_t largest;
   bson_decimal128_t tiniest;
   bson_decimal128_t full_house;

   {
       char buf[] = "12345689012345789012345";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, large);
   }
   {
       char buf[] = "1234567890123456789012345678901234";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, all_digits);
   }
   {
       char buf[] = "9.999999999999999999999999999999999E+6144";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, largest);
   }
   {
       char buf[] = "9.999999999999999999999999999999999E-6143";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, tiniest);
   }
   {
       char buf[] = "5.192296858534827628530496329220095E+33";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, full_house);
   }

   CHECK (large == bson_decimal128_t(0x304000000000029d, 0x42da3a76f9e0d979));
   CHECK (all_digits == bson_decimal128_t(0x30403cde6fff9732, 0xde825cd07e96aff2));
   CHECK (largest == bson_decimal128_t(0x5fffed09bead87c0, 0x378d8e63ffffffff));
   CHECK (tiniest == bson_decimal128_t(0x0001ed09bead87c0, 0x378d8e63ffffffff));
   CHECK (full_house == bson_decimal128_t(0x3040ffffffffffff, 0xffffffffffffffff));


   {
       char buf[] = "12345689012345789012345";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, large);
   }
   {
       char buf[] = "1234567890123456789012345678901234";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, all_digits);
   }
   {
       char buf[] = "9.999999999999999999999999999999999E+6144";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, largest);
   }
   {
       char buf[] = "9.999999999999999999999999999999999E-6143";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, tiniest);
   }
   {
       char buf[] = "5.192296858534827628530496329220095E+33";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, full_house);
   }

   CHECK (large == bson_decimal128_t(0x304000000000029d, 0x42da3a76f9e0d979));
   CHECK (all_digits == bson_decimal128_t(0x30403cde6fff9732, 0xde825cd07e96aff2));
   CHECK (largest == bson_decimal128_t(0x5fffed09bead87c0, 0x378d8e63ffffffff));
   CHECK (tiniest == bson_decimal128_t(0x0001ed09bead87c0, 0x378d8e63ffffffff));
   CHECK (full_house == bson_decimal128_t(0x3040ffffffffffff, 0xffffffffffffffff));
}


TEST_CASE("test_decimal128_from_string__exponent_normalization")
{
   bson_decimal128_t trailing_zeros;
   bson_decimal128_t one_normalize;
   bson_decimal128_t no_normalize;
   bson_decimal128_t a_disaster;

   {
       char buf[] = "1000000000000000000000000000000000000000";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, trailing_zeros);
   }
   {
       char buf[] = "10000000000000000000000000000000000";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, one_normalize);
   }
   {
       char buf[] = "1000000000000000000000000000000000";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, no_normalize);
   }
   {
       char buf[] = "100000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "0000000000000000000000000000000000";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, a_disaster);
   }

   CHECK (trailing_zeros == bson_decimal128_t(0x304c314dc6448d93, 0x38c15b0a00000000));
   CHECK (one_normalize == bson_decimal128_t(0x3042314dc6448d93, 0x38c15b0a00000000));
   CHECK (no_normalize == bson_decimal128_t(0x3040314dc6448d93, 0x38c15b0a00000000));
   CHECK (a_disaster == bson_decimal128_t(0x37cc314dc6448d93, 0x38c15b0a00000000));


   {
       char buf[] = "1000000000000000000000000000000000000000";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, trailing_zeros);
   }
   {
       char buf[] = "10000000000000000000000000000000000";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, one_normalize);
   }
   {
       char buf[] = "1000000000000000000000000000000000";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, no_normalize);
   }
   {
       char buf[] = "100000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000"
          "0000000000000000000000000000000000";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, a_disaster);

   }
   CHECK (trailing_zeros == bson_decimal128_t(0x304c314dc6448d93, 0x38c15b0a00000000));
   CHECK (one_normalize == bson_decimal128_t(0x3042314dc6448d93, 0x38c15b0a00000000));
   CHECK (no_normalize == bson_decimal128_t(0x3040314dc6448d93, 0x38c15b0a00000000));
   CHECK (a_disaster == bson_decimal128_t(0x37cc314dc6448d93, 0x38c15b0a00000000));
}


TEST_CASE("test_decimal128_from_string__zeros")
{
   bson_decimal128_t zero;
   bson_decimal128_t exponent_zero;
   bson_decimal128_t large_exponent;
   bson_decimal128_t negative_zero;

   {
       char buf[] = "0";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, zero);
   }
   {
       char buf[] = "0e-611";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, exponent_zero);
   }
   {
       char buf[] = "0e+6000";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, large_exponent);
   }
   {
       char buf[] = "-0e-1";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, negative_zero);
   }

   CHECK (zero == bson_decimal128_t(0x3040000000000000, 0x0000000000000000));
   CHECK (exponent_zero == bson_decimal128_t(0x2b7a000000000000, 0x0000000000000000));
   CHECK (large_exponent == bson_decimal128_t(0x5f20000000000000, 0x0000000000000000));
   CHECK (negative_zero == bson_decimal128_t(0xb03e000000000000, 0x0000000000000000));


   {
       char buf[] = "0";
       decimal128_from_chars(buf, buf+1, zero);
   }
   {
       char buf[] = "0e-611";
       decimal128_from_chars(buf, buf+sizeof(buf)-1, exponent_zero);
   }
   {
       char buf[] = "0e+6000";
       decimal128_from_chars(buf, buf+7, large_exponent);
   }
   {
       char buf[] = "-0e-1";
       decimal128_from_chars(buf, buf+5, negative_zero);
   }

   CHECK (zero == bson_decimal128_t(0x3040000000000000, 0x0000000000000000));
   CHECK (exponent_zero == bson_decimal128_t(0x2b7a000000000000, 0x0000000000000000));
   CHECK (large_exponent == bson_decimal128_t(0x5f20000000000000, 0x0000000000000000));
   CHECK (negative_zero == bson_decimal128_t(0xb03e000000000000, 0x0000000000000000));
}

#if 0
TEST_CASE("test_decimal128_from_string_w_len__special")
{
   bson_decimal128_t number;
   bson_decimal128_t number_two;
   bson_decimal128_t negative_number;

   /* These strings have more bytes than the length indicates. */
   {
       char buf[] = "12345678901234567abcd";
       decimal128_from_chars(buf, buf+sizeof(buf)-1 17, number);
   }
   {
       char buf[] = "989898983458abcd";
       decimal128_from_chars(buf, buf+sizeof(buf)-1 12, number_two);
   }
   {
       char buf[] = "-12345678901234567abcd";
       decimal128_from_chars(buf, buf+sizeof(buf)-1 18, negative_number);
   }

   CHECK (number == bson_decimal128_t(0x3040000000000000, 0x002bdc545d6b4b87));
   CHECK (number_two == bson_decimal128_t(0x3040000000000000, 0x000000e67a93c822));
   CHECK (negative_number == bson_decimal128_t(0xb040000000000000, 0x002bdc545d6b4b87));
}
#endif
