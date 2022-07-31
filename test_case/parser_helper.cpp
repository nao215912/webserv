#include "gtest/gtest.h"
#include <vector>
#include "../../src/communication/ParserHelper.hpp"

// [find_crlf]

TEST(parser_helper_find_crlf, basic) {
    const HTTP::byte_string str = HTTP::strfy("012\r456\n890\r\r345\r\r\r901\r\n\r567\n\r\n123\n\n\n");
    EXPECT_EQ(IndexRange(7, 8), ParserHelper::find_crlf(str, 0, str.size()));
    EXPECT_EQ(IndexRange(6, 5), ParserHelper::find_crlf(str, 0, 5));
    EXPECT_EQ(IndexRange(7, 8), ParserHelper::find_crlf(str, 7, str.size()));
    EXPECT_EQ(IndexRange(22, 24), ParserHelper::find_crlf(str, 10, str.size()));
    EXPECT_EQ(IndexRange(36, 37), ParserHelper::find_crlf(str, 36, str.size()));
    EXPECT_EQ(IndexRange(38, 37), ParserHelper::find_crlf(str, 37, str.size()));
}

TEST(parser_helper_find_crlf, backward) {
    const HTTP::byte_string str = HTTP::strfy("\r\n\n");
    EXPECT_EQ(IndexRange(0, 2), ParserHelper::find_crlf(str, 0, str.size()));
    EXPECT_EQ(IndexRange(2, 1), ParserHelper::find_crlf(str, 0, 1));
    EXPECT_EQ(IndexRange(0, 2), ParserHelper::find_crlf(str, 1, str.size()));
    EXPECT_EQ(IndexRange(2, 3), ParserHelper::find_crlf(str, 2, str.size()));
}

TEST(parser_helper_find_crlf, nothing) {
    const HTTP::byte_string str = HTTP::strfy("aiueo");
    EXPECT_EQ(IndexRange(1, 0), ParserHelper::find_crlf(str, 0, 0));
    EXPECT_EQ(IndexRange(str.size() + 1, str.size()), ParserHelper::find_crlf(str, 0, str.size()));
}

// [find_crlf_header_value]

// [find_obs_fold]

// [ignore_crlf]

// [is_sp]
TEST(parser_helper_is_sp, is_sp) {
    EXPECT_EQ(true, ParserHelper::is_sp(ParserHelper::SP[0]));
    EXPECT_EQ(false, ParserHelper::is_sp('\t'));
    EXPECT_EQ(false, ParserHelper::is_sp('a'));
    EXPECT_EQ(false, ParserHelper::is_sp('\v'));
    EXPECT_EQ(false, ParserHelper::is_sp('\n'));
    EXPECT_EQ(false, ParserHelper::is_sp('\r'));
}

// [ignore_sp]

// [ignore_not_sp]

// [find_leading_crlf]

// [split_by_sp]

// [split]

// [normalize_header_key]

// [xtou]

// [str_to_u]
TEST(parser_helper_str_to_u, is_0) {
    const HTTP::byte_string str = HTTP::strfy("0");
    std::pair<bool, unsigned int> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(true, res.first);
    EXPECT_EQ(0, res.second);
}

TEST(parser_helper_str_to_u, is_1) {
    const HTTP::byte_string str = HTTP::strfy("1");
    std::pair<bool, unsigned int> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(true, res.first);
    EXPECT_EQ(1, res.second);
}

TEST(parser_helper_str_to_u, is_UINT_MAX_less_1) {
    const HTTP::byte_string str = HTTP::strfy("4294967294");
    std::pair<bool, unsigned int> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(true, res.first);
    EXPECT_EQ(UINT_MAX - 1, res.second);
}

TEST(parser_helper_str_to_u, is_UINT_MAX) {
    const HTTP::byte_string str = HTTP::strfy("4294967295");
    std::pair<bool, unsigned int> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(true, res.first);
    EXPECT_EQ(UINT_MAX, res.second);
}

TEST(parser_helper_str_to_u, is_UINT_MAX_with_leading_0) {
    const HTTP::byte_string str = HTTP::strfy("0000000004294967295");
    std::pair<bool, unsigned int> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(false, res.first);
}

TEST(parser_helper_str_to_u, is_UINT_MAX_plus_1) {
    const HTTP::byte_string str = HTTP::strfy("4294967296");
    std::pair<bool, unsigned int> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(false, res.first);
}

TEST(parser_helper_str_to_u, is_minus_1) {
    const HTTP::byte_string str = HTTP::strfy("-1");
    std::pair<bool, unsigned int> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(false, res.first);
}

// [utos]
TEST(parser_helper_utos, from_0) {
    EXPECT_EQ(HTTP::strfy("0"), ParserHelper::utos(0, 8));
    EXPECT_EQ(HTTP::strfy("0"), ParserHelper::utos(0, 10));
    EXPECT_EQ(HTTP::strfy("0"), ParserHelper::utos(0, 16));
}

TEST(parser_helper_utos, from_1) {
    EXPECT_EQ(HTTP::strfy("1"), ParserHelper::utos(1, 8));
    EXPECT_EQ(HTTP::strfy("1"), ParserHelper::utos(1, 10));
    EXPECT_EQ(HTTP::strfy("1"), ParserHelper::utos(1, 16));
}

TEST(parser_helper_utos, from_8) {
    EXPECT_EQ(HTTP::strfy("10"), ParserHelper::utos(8, 8));
    EXPECT_EQ(HTTP::strfy("8"), ParserHelper::utos(8, 10));
    EXPECT_EQ(HTTP::strfy("8"), ParserHelper::utos(8, 16));
}

TEST(parser_helper_utos, from_10) {
    EXPECT_EQ(HTTP::strfy("12"), ParserHelper::utos(10, 8));
    EXPECT_EQ(HTTP::strfy("10"), ParserHelper::utos(10, 10));
    EXPECT_EQ(HTTP::strfy("a"), ParserHelper::utos(10, 16));
}

TEST(parser_helper_utos, from_16) {
    EXPECT_EQ(HTTP::strfy("20"), ParserHelper::utos(16, 8));
    EXPECT_EQ(HTTP::strfy("16"), ParserHelper::utos(16, 10));
    EXPECT_EQ(HTTP::strfy("10"), ParserHelper::utos(16, 16));
}

TEST(parser_helper_utos, from_UINT_MAX) {
    EXPECT_EQ(HTTP::strfy("37777777776"), ParserHelper::utos(UINT_MAX - 1, 8));
    EXPECT_EQ(HTTP::strfy("4294967294"), ParserHelper::utos(UINT_MAX - 1, 10));
    EXPECT_EQ(HTTP::strfy("fffffffe"), ParserHelper::utos(UINT_MAX - 1, 16));
}

TEST(parser_helper_utos, from_UINT_MAX_minus_1) {
    EXPECT_EQ(HTTP::strfy("37777777777"), ParserHelper::utos(UINT_MAX, 8));
    EXPECT_EQ(HTTP::strfy("4294967295"), ParserHelper::utos(UINT_MAX, 10));
    EXPECT_EQ(HTTP::strfy("ffffffff"), ParserHelper::utos(UINT_MAX, 16));
}

// [quality_to_u]

// [extract_quoted_or_token]

