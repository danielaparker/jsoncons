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
#include "bson_decimal128.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>
#include <array>
#include <catch/catch.hpp>

using namespace jsoncons;
using namespace jsoncons::bson;

#define DECIMAL128_FROM_ULLS(dec, h, l) \
   do {                                 \
      (dec).high = (h);                 \
      (dec).low = (l);                  \
   } while (0);


TEST_CASE("test_decimal128_to_string__infinity")
{
   bson_decimal128_t positive_infinity;
   bson_decimal128_t negative_infinity;
   char bid_string[decimal128_limits::max_digits10+1];

   DECIMAL128_FROM_ULLS (positive_infinity, 0x7800000000000000, 0);
   DECIMAL128_FROM_ULLS (negative_infinity, 0xf800000000000000, 0);

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &positive_infinity);
   CHECK (!strcmp (bid_string, "Infinity"));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &negative_infinity);
   CHECK (!strcmp (bid_string, "-Infinity"));
}


TEST_CASE("test_decimal128_to_string__nan")
{
   bson_decimal128_t dec_pnan;
   bson_decimal128_t dec_nnan;
   bson_decimal128_t dec_psnan;
   bson_decimal128_t dec_nsnan;
   bson_decimal128_t dec_payload_nan;

   /* All the above should just be NaN. */
   char bid_string[decimal128_limits::max_digits10+1];

   DECIMAL128_FROM_ULLS (dec_pnan, 0x7c00000000000000, 0);
   DECIMAL128_FROM_ULLS (dec_nnan, 0xfc00000000000000, 0);
   DECIMAL128_FROM_ULLS (dec_psnan, 0x7e00000000000000, 0);
   DECIMAL128_FROM_ULLS (dec_nsnan, 0xfe00000000000000, 0);
   DECIMAL128_FROM_ULLS (dec_payload_nan, 0x7e00000000000000, 12);

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &dec_pnan);
   CHECK (!strcmp (bid_string, "NaN"));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &dec_nnan);
   CHECK (!strcmp (bid_string, "NaN"));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &dec_psnan);
   CHECK (!strcmp (bid_string, "NaN"));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &dec_nsnan);
   CHECK (!strcmp (bid_string, "NaN"));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &dec_payload_nan);
   CHECK (!strcmp (bid_string, "NaN"));
}


TEST_CASE("test_decimal128_to_string__regular")
{
   char bid_string[decimal128_limits::max_digits10+1];
   bson_decimal128_t one;
   bson_decimal128_t zero;
   bson_decimal128_t two;
   bson_decimal128_t negative_one;
   bson_decimal128_t negative_zero;
   bson_decimal128_t tenth;
   bson_decimal128_t smallest_regular;
   bson_decimal128_t largest_regular;
   bson_decimal128_t trailing_zeros;
   bson_decimal128_t all_digits;
   bson_decimal128_t full_house;

   DECIMAL128_FROM_ULLS (one, 0x3040000000000000, 0x0000000000000001);
   DECIMAL128_FROM_ULLS (zero, 0x3040000000000000, 0x0000000000000000);
   DECIMAL128_FROM_ULLS (two, 0x3040000000000000, 0x0000000000000002);
   DECIMAL128_FROM_ULLS (negative_one, 0xb040000000000000, 0x0000000000000001);
   DECIMAL128_FROM_ULLS (negative_zero, 0xb040000000000000, 0x0000000000000000);
   DECIMAL128_FROM_ULLS (
      tenth, 0x303e000000000000, 0x0000000000000001); /* 0.1 */
   /* 0.001234 */
   DECIMAL128_FROM_ULLS (
      smallest_regular, 0x3034000000000000, 0x00000000000004d2);
   /* 12345789012 */
   DECIMAL128_FROM_ULLS (
      largest_regular, 0x3040000000000000, 0x0000001cbe991a14);
   /* 0.00123400000 */
   DECIMAL128_FROM_ULLS (
      trailing_zeros, 0x302a000000000000, 0x00000000075aef40);
   /* 0.1234567890123456789012345678901234 */
   DECIMAL128_FROM_ULLS (all_digits, 0x2ffc3cde6fff9732, 0xde825cd07e96aff2);

   /* 5192296858534827628530496329220095 */
   DECIMAL128_FROM_ULLS (full_house, 0x3040ffffffffffff, 0xffffffffffffffff);

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &one);
   CHECK (!strcmp ("1", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &zero);
   CHECK (!strcmp ("0", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &two);
   CHECK (!strcmp ("2", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &negative_one);
   CHECK (!strcmp ("-1", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &negative_zero);
   CHECK (!strcmp ("-0", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &tenth);
   CHECK (!strcmp ("0.1", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &smallest_regular);
   CHECK (!strcmp ("0.001234", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &largest_regular);
   CHECK (!strcmp ("123456789012", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &trailing_zeros);
   CHECK (!strcmp ("0.00123400000", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &all_digits);
   CHECK (!strcmp ("0.1234567890123456789012345678901234", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &full_house);
   CHECK (!strcmp ("5192296858534827628530496329220095", bid_string));
}


TEST_CASE("test_decimal128_to_string__scientific")
{
   char bid_string[decimal128_limits::max_digits10+1];

   bson_decimal128_t huge;     /* 1.000000000000000000000000000000000E+6144 */
   bson_decimal128_t tiny;     /* 1E-6176 */
   bson_decimal128_t neg_tiny; /* -1E-6176 */
   bson_decimal128_t large;    /* 9.999987654321E+112 */
   bson_decimal128_t largest;  /* 9.999999999999999999999999999999999E+6144 */
   bson_decimal128_t tiniest;  /* 9.999999999999999999999999999999999E-6143 */
   bson_decimal128_t trailing_zero;            /* 1.050E9 */
   bson_decimal128_t one_trailing_zero;        /* 1.050E4 */
   bson_decimal128_t move_decimal;             /* 105 */
   bson_decimal128_t move_decimal_after;       /* 1.05E3 */
   bson_decimal128_t trailing_zero_no_decimal; /* 1E3 */

   DECIMAL128_FROM_ULLS (huge, 0x5ffe314dc6448d93, 0x38c15b0a00000000);
   DECIMAL128_FROM_ULLS (tiny, 0x0000000000000000, 0x0000000000000001);
   DECIMAL128_FROM_ULLS (neg_tiny, 0x8000000000000000, 0x0000000000000001);
   DECIMAL128_FROM_ULLS (large, 0x3108000000000000, 0x000009184db63eb1);
   DECIMAL128_FROM_ULLS (largest, 0x5fffed09bead87c0, 0x378d8e63ffffffff);
   DECIMAL128_FROM_ULLS (tiniest, 0x0001ed09bead87c0, 0x378d8e63ffffffff);
   DECIMAL128_FROM_ULLS (trailing_zero, 0x304c000000000000, 0x000000000000041a);
   DECIMAL128_FROM_ULLS (
      one_trailing_zero, 0x3042000000000000, 0x000000000000041a);
   DECIMAL128_FROM_ULLS (move_decimal, 0x3040000000000000, 0x0000000000000069);
   DECIMAL128_FROM_ULLS (
      move_decimal_after, 0x3042000000000000, 0x0000000000000069);
   DECIMAL128_FROM_ULLS (
      trailing_zero_no_decimal, 0x3046000000000000, 0x0000000000000001);

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &huge);
   CHECK (
      !strcmp ("1.000000000000000000000000000000000E+6144", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &tiny);
   CHECK (!strcmp ("1E-6176", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &neg_tiny);
   CHECK (!strcmp ("-1E-6176", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &neg_tiny);
   CHECK (!strcmp ("-1E-6176", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &large);
   CHECK (!strcmp ("9.999987654321E+112", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &largest);
   CHECK (
      !strcmp ("9.999999999999999999999999999999999E+6144", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &tiniest);
   CHECK (
      !strcmp ("9.999999999999999999999999999999999E-6143", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &trailing_zero);
   CHECK (!strcmp ("1.050E+9", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &one_trailing_zero);
   CHECK (!strcmp ("1.050E+4", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &move_decimal);
   CHECK (!strcmp ("105", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &move_decimal_after);
   CHECK (!strcmp ("1.05E+3", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &trailing_zero_no_decimal);
   CHECK (!strcmp ("1E+3", bid_string));
}


TEST_CASE("test_decimal128_to_string__zeros")
{
   char bid_string[decimal128_limits::max_digits10+1];

   bson_decimal128_t zero;         /* 0 */
   bson_decimal128_t pos_exp_zero; /* 0E+300 */
   bson_decimal128_t neg_exp_zero; /* 0E-600 */

   DECIMAL128_FROM_ULLS (zero, 0x3040000000000000, 0x0000000000000000);
   DECIMAL128_FROM_ULLS (pos_exp_zero, 0x3298000000000000, 0x0000000000000000);
   DECIMAL128_FROM_ULLS (neg_exp_zero, 0x2b90000000000000, 0x0000000000000000);

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &zero);
   CHECK (!strcmp ("0", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &pos_exp_zero);
   CHECK (!strcmp ("0E+300", bid_string));

   decimal128_to_chars(bid_string, bid_string+sizeof(bid_string), &neg_exp_zero);
   CHECK (!strcmp ("0E-600", bid_string));
}


#define IS_NAN(dec) (dec).high == 0x7c00000000000000ull


TEST_CASE("test_decimal128_from_string__invalid_inputs")
{
   bson_decimal128_t dec;

   decimal128_from_chars (".", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars (".e", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("invalid", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("in", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("i", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("E02", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("..1", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("1abcede", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("1.24abc", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("1.24abcE+02", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("1.24E+02abc2d", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("E+02", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("e+02", &dec);
   CHECK (IS_NAN (dec));

   bson_decimal128_from_string_w_len (".", 1, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len (".e", 2, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("", 0, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("invalid", 7, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("in", 2, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("i", 1, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("E02", 3, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("..1", 3, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("1abcede", 7, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("1.24abc", 7, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("1.24abcE+02", 11, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("1.24E+02abc2d", 13, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("E+02", 4, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("e+02", 4, &dec);
   CHECK (IS_NAN (dec));
}


TEST_CASE("test_decimal128_from_string__nan")
{
   bson_decimal128_t dec;

   decimal128_from_chars ("NaN", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("+NaN", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("-NaN", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("-nan", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("1e", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("+nan", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("nan", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("Nan", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("+Nan", &dec);
   CHECK (IS_NAN (dec));
   decimal128_from_chars ("-Nan", &dec);
   CHECK (IS_NAN (dec));

   bson_decimal128_from_string_w_len ("NaN", 3, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("+NaN", 4, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("-NaN", 4, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("-nan", 4, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("1e", 2, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("+nan", 4, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("nan", 3, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("Nan", 3, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("+Nan", 4, &dec);
   CHECK (IS_NAN (dec));
   bson_decimal128_from_string_w_len ("-Nan", 4, &dec);
   CHECK (IS_NAN (dec));
}


#define IS_PINFINITY(dec) (dec).high == 0x7800000000000000
#define IS_NINFINITY(dec) (dec).high == 0xf800000000000000


TEST_CASE("test_decimal128_from_string__infinity")
{
   bson_decimal128_t dec;

   decimal128_from_chars ("Infinity", &dec);
   CHECK (IS_PINFINITY (dec));
   decimal128_from_chars ("+Infinity", &dec);
   CHECK (IS_PINFINITY (dec));
   decimal128_from_chars ("+Inf", &dec);
   CHECK (IS_PINFINITY (dec));
   decimal128_from_chars ("-Inf", &dec);
   CHECK (IS_NINFINITY (dec));
   decimal128_from_chars ("-Infinity", &dec);
   CHECK (IS_NINFINITY (dec));

   bson_decimal128_from_string_w_len ("Infinity", 8, &dec);
   CHECK (IS_PINFINITY (dec));
   bson_decimal128_from_string_w_len ("+Infinity", 9, &dec);
   CHECK (IS_PINFINITY (dec));
   bson_decimal128_from_string_w_len ("+Inf", 4, &dec);
   CHECK (IS_PINFINITY (dec));
   bson_decimal128_from_string_w_len ("-Inf", 4, &dec);
   CHECK (IS_NINFINITY (dec));
   bson_decimal128_from_string_w_len ("-Infinity", 9, &dec);
   CHECK (IS_NINFINITY (dec));
}


static bool
decimal128_equal (bson_decimal128_t *dec, uint64_t high, uint64_t low)
{
   bson_decimal128_t test;
   DECIMAL128_FROM_ULLS (test, high, low);
   return memcmp (dec, &test, sizeof (*dec)) == 0;
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

   decimal128_from_chars ("1", &one);
   decimal128_from_chars ("-1", &negative_one);
   decimal128_from_chars ("0", &zero);
   decimal128_from_chars ("-0", &negative_zero);
   decimal128_from_chars ("12345678901234567", &number);
   decimal128_from_chars ("989898983458", &number_two);
   decimal128_from_chars ("-12345678901234567", &negative_number);

   decimal128_from_chars ("0.12345", &fractional_number);
   decimal128_from_chars ("0.0012345", &leading_zeros);

   decimal128_from_chars ("00012345678901234567",
                                &leading_insignificant_zeros);

   CHECK (
      decimal128_equal (&one, 0x3040000000000000, 0x0000000000000001));
   CHECK (
      decimal128_equal (&negative_one, 0xb040000000000000, 0x0000000000000001));
   CHECK (
      decimal128_equal (&zero, 0x3040000000000000, 0x0000000000000000));
   CHECK (decimal128_equal (
      &negative_zero, 0xb040000000000000, 0x0000000000000000));
   CHECK (
      decimal128_equal (&number, 0x3040000000000000, 0x002bdc545d6b4b87));
   CHECK (
      decimal128_equal (&number_two, 0x3040000000000000, 0x000000e67a93c822));
   CHECK (decimal128_equal (
      &negative_number, 0xb040000000000000, 0x002bdc545d6b4b87));
   CHECK (decimal128_equal (
      &fractional_number, 0x3036000000000000, 0x0000000000003039));
   CHECK (decimal128_equal (
      &leading_zeros, 0x3032000000000000, 0x0000000000003039));
   CHECK (decimal128_equal (
      &leading_insignificant_zeros, 0x3040000000000000, 0x002bdc545d6b4b87));


   bson_decimal128_from_string_w_len ("1", 1, &one);
   bson_decimal128_from_string_w_len ("-1", 2, &negative_one);
   bson_decimal128_from_string_w_len ("0", 1, &zero);
   bson_decimal128_from_string_w_len ("-0", 2, &negative_zero);
   bson_decimal128_from_string_w_len ("12345678901234567", 17, &number);
   bson_decimal128_from_string_w_len ("989898983458", 12, &number_two);
   bson_decimal128_from_string_w_len (
      "-12345678901234567", 18, &negative_number);

   bson_decimal128_from_string_w_len ("0.12345", 7, &fractional_number);
   bson_decimal128_from_string_w_len ("0.0012345", 9, &leading_zeros);

   bson_decimal128_from_string_w_len (
      "00012345678901234567", 20, &leading_insignificant_zeros);

   CHECK (
      decimal128_equal (&one, 0x3040000000000000, 0x0000000000000001));
   CHECK (
      decimal128_equal (&negative_one, 0xb040000000000000, 0x0000000000000001));
   CHECK (
      decimal128_equal (&zero, 0x3040000000000000, 0x0000000000000000));
   CHECK (decimal128_equal (
      &negative_zero, 0xb040000000000000, 0x0000000000000000));
   CHECK (
      decimal128_equal (&number, 0x3040000000000000, 0x002bdc545d6b4b87));
   CHECK (
      decimal128_equal (&number_two, 0x3040000000000000, 0x000000e67a93c822));
   CHECK (decimal128_equal (
      &negative_number, 0xb040000000000000, 0x002bdc545d6b4b87));
   CHECK (decimal128_equal (
      &fractional_number, 0x3036000000000000, 0x0000000000003039));
   CHECK (decimal128_equal (
      &leading_zeros, 0x3032000000000000, 0x0000000000003039));
   CHECK (decimal128_equal (
      &leading_insignificant_zeros, 0x3040000000000000, 0x002bdc545d6b4b87));
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

   decimal128_from_chars ("10e0", &ten);
   decimal128_from_chars ("1e1", &ten_again);
   decimal128_from_chars ("10e-1", &one);

   CHECK (
      decimal128_equal (&ten, 0x3040000000000000, 0x000000000000000a));
   CHECK (
      decimal128_equal (&ten_again, 0x3042000000000000, 0x0000000000000001));
   CHECK (
      decimal128_equal (&one, 0x303e000000000000, 0x000000000000000a));

   decimal128_from_chars ("12345678901234567e6111", &huge_exp);
   decimal128_from_chars ("1e-6176", &tiny_exp);

   CHECK (
      decimal128_equal (&huge_exp, 0x5ffe000000000000, 0x002bdc545d6b4b87));
   CHECK (
      decimal128_equal (&tiny_exp, 0x0000000000000000, 0x0000000000000001));

   decimal128_from_chars ("-100E-10", &fractional);
   decimal128_from_chars ("10.50E8", &trailing_zeros);

   CHECK (
      decimal128_equal (&fractional, 0xb02c000000000000, 0x0000000000000064));
   CHECK (decimal128_equal (
      &trailing_zeros, 0x304c000000000000, 0x000000000000041a));


   bson_decimal128_from_string_w_len ("10e0", 4, &ten);
   bson_decimal128_from_string_w_len ("1e1", 3, &ten_again);
   bson_decimal128_from_string_w_len ("10e-1", 5, &one);

   CHECK (
      decimal128_equal (&ten, 0x3040000000000000, 0x000000000000000a));
   CHECK (
      decimal128_equal (&ten_again, 0x3042000000000000, 0x0000000000000001));
   CHECK (
      decimal128_equal (&one, 0x303e000000000000, 0x000000000000000a));

   bson_decimal128_from_string_w_len ("12345678901234567e6111", 22, &huge_exp);
   bson_decimal128_from_string_w_len ("1e-6176", 7, &tiny_exp);

   CHECK (
      decimal128_equal (&huge_exp, 0x5ffe000000000000, 0x002bdc545d6b4b87));
   CHECK (
      decimal128_equal (&tiny_exp, 0x0000000000000000, 0x0000000000000001));

   bson_decimal128_from_string_w_len ("-100E-10", 8, &fractional);
   bson_decimal128_from_string_w_len ("10.50E8", 7, &trailing_zeros);

   CHECK (
      decimal128_equal (&fractional, 0xb02c000000000000, 0x0000000000000064));
   CHECK (decimal128_equal (
      &trailing_zeros, 0x304c000000000000, 0x000000000000041a));
}


TEST_CASE("test_decimal128_from_string__large")
{
   bson_decimal128_t large;
   bson_decimal128_t all_digits;
   bson_decimal128_t largest;
   bson_decimal128_t tiniest;
   bson_decimal128_t full_house;

   decimal128_from_chars ("12345689012345789012345", &large);
   decimal128_from_chars ("1234567890123456789012345678901234",
                                &all_digits);
   decimal128_from_chars ("9.999999999999999999999999999999999E+6144",
                                &largest);
   decimal128_from_chars ("9.999999999999999999999999999999999E-6143",
                                &tiniest);
   decimal128_from_chars ("5.192296858534827628530496329220095E+33",
                                &full_house);

   CHECK (
      decimal128_equal (&large, 0x304000000000029d, 0x42da3a76f9e0d979));
   CHECK (
      decimal128_equal (&all_digits, 0x30403cde6fff9732, 0xde825cd07e96aff2));
   CHECK (
      decimal128_equal (&largest, 0x5fffed09bead87c0, 0x378d8e63ffffffff));
   CHECK (
      decimal128_equal (&tiniest, 0x0001ed09bead87c0, 0x378d8e63ffffffff));
   CHECK (
      decimal128_equal (&full_house, 0x3040ffffffffffff, 0xffffffffffffffff));


   bson_decimal128_from_string_w_len ("12345689012345789012345", -1, &large);
   bson_decimal128_from_string_w_len (
      "1234567890123456789012345678901234", -1, &all_digits);
   bson_decimal128_from_string_w_len (
      "9.999999999999999999999999999999999E+6144", -1, &largest);
   bson_decimal128_from_string_w_len (
      "9.999999999999999999999999999999999E-6143", -1, &tiniest);
   bson_decimal128_from_string_w_len (
      "5.192296858534827628530496329220095E+33", -1, &full_house);

   CHECK (
      decimal128_equal (&large, 0x304000000000029d, 0x42da3a76f9e0d979));
   CHECK (
      decimal128_equal (&all_digits, 0x30403cde6fff9732, 0xde825cd07e96aff2));
   CHECK (
      decimal128_equal (&largest, 0x5fffed09bead87c0, 0x378d8e63ffffffff));
   CHECK (
      decimal128_equal (&tiniest, 0x0001ed09bead87c0, 0x378d8e63ffffffff));
   CHECK (
      decimal128_equal (&full_house, 0x3040ffffffffffff, 0xffffffffffffffff));
}


TEST_CASE("test_decimal128_from_string__exponent_normalization")
{
   bson_decimal128_t trailing_zeros;
   bson_decimal128_t one_normalize;
   bson_decimal128_t no_normalize;
   bson_decimal128_t a_disaster;

   decimal128_from_chars ("1000000000000000000000000000000000000000",
                                &trailing_zeros);
   decimal128_from_chars ("10000000000000000000000000000000000",
                                &one_normalize);
   decimal128_from_chars ("1000000000000000000000000000000000",
                                &no_normalize);
   decimal128_from_chars (
      "100000000000000000000000000000000000000000000000000000000000000000000"
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
      "0000000000000000000000000000000000",
      &a_disaster);

   CHECK (decimal128_equal (
      &trailing_zeros, 0x304c314dc6448d93, 0x38c15b0a00000000));
   CHECK (decimal128_equal (
      &one_normalize, 0x3042314dc6448d93, 0x38c15b0a00000000));
   CHECK (
      decimal128_equal (&no_normalize, 0x3040314dc6448d93, 0x38c15b0a00000000));
   CHECK (
      decimal128_equal (&a_disaster, 0x37cc314dc6448d93, 0x38c15b0a00000000));


   bson_decimal128_from_string_w_len (
      "1000000000000000000000000000000000000000", -1, &trailing_zeros);
   bson_decimal128_from_string_w_len (
      "10000000000000000000000000000000000", -1, &one_normalize);
   bson_decimal128_from_string_w_len (
      "1000000000000000000000000000000000", -1, &no_normalize);
   bson_decimal128_from_string_w_len (
      "100000000000000000000000000000000000000000000000000000000000000000000"
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
      "0000000000000000000000000000000000",
      -1,
      &a_disaster);

   CHECK (decimal128_equal (
      &trailing_zeros, 0x304c314dc6448d93, 0x38c15b0a00000000));
   CHECK (decimal128_equal (
      &one_normalize, 0x3042314dc6448d93, 0x38c15b0a00000000));
   CHECK (
      decimal128_equal (&no_normalize, 0x3040314dc6448d93, 0x38c15b0a00000000));
   CHECK (
      decimal128_equal (&a_disaster, 0x37cc314dc6448d93, 0x38c15b0a00000000));
}


TEST_CASE("test_decimal128_from_string__zeros")
{
   bson_decimal128_t zero;
   bson_decimal128_t exponent_zero;
   bson_decimal128_t large_exponent;
   bson_decimal128_t negative_zero;

   decimal128_from_chars ("0", &zero);
   decimal128_from_chars ("0e-611", &exponent_zero);
   decimal128_from_chars ("0e+6000", &large_exponent);
   decimal128_from_chars ("-0e-1", &negative_zero);

   CHECK (
      decimal128_equal (&zero, 0x3040000000000000, 0x0000000000000000));
   CHECK (decimal128_equal (
      &exponent_zero, 0x2b7a000000000000, 0x0000000000000000));
   CHECK (decimal128_equal (
      &large_exponent, 0x5f20000000000000, 0x0000000000000000));
   CHECK (decimal128_equal (
      &negative_zero, 0xb03e000000000000, 0x0000000000000000));


   bson_decimal128_from_string_w_len ("0", 1, &zero);
   bson_decimal128_from_string_w_len ("0e-611", -1, &exponent_zero);
   bson_decimal128_from_string_w_len ("0e+6000", 7, &large_exponent);
   bson_decimal128_from_string_w_len ("-0e-1", 5, &negative_zero);

   CHECK (
      decimal128_equal (&zero, 0x3040000000000000, 0x0000000000000000));
   CHECK (decimal128_equal (
      &exponent_zero, 0x2b7a000000000000, 0x0000000000000000));
   CHECK (decimal128_equal (
      &large_exponent, 0x5f20000000000000, 0x0000000000000000));
   CHECK (decimal128_equal (
      &negative_zero, 0xb03e000000000000, 0x0000000000000000));
}

TEST_CASE("test_decimal128_from_string_w_len__special")
{
   bson_decimal128_t number;
   bson_decimal128_t number_two;
   bson_decimal128_t negative_number;

   /* These strings have more bytes than the length indicates. */
   bson_decimal128_from_string_w_len ("12345678901234567abcd", 17, &number);
   bson_decimal128_from_string_w_len ("989898983458abcd", 12, &number_two);
   bson_decimal128_from_string_w_len (
      "-12345678901234567abcd", 18, &negative_number);

   CHECK (
      decimal128_equal (&number, 0x3040000000000000, 0x002bdc545d6b4b87));
   CHECK (
      decimal128_equal (&number_two, 0x3040000000000000, 0x000000e67a93c822));
   CHECK (decimal128_equal (
      &negative_number, 0xb040000000000000, 0x002bdc545d6b4b87));
}

