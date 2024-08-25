//
// Created by Elijah Crain on 8/21/24.
//
#include <gtest/gtest.h>
#include "../Compiler/Lexer.cpp"


TEST(LexerTest, ParseIntPureNum) {
  std::string_view pureNum = "1234";
  constexpr std::string_view pureNumPost;
  const std::string ret = Lexer::parseInt(pureNum);
  EXPECT_EQ("1234", ret);
  EXPECT_EQ(pureNumPost, pureNum);
}

TEST(LexerTest, ParseIntNumThenAlpha) {
  std::string_view numThenAlpha = "1234f";
  constexpr std::string_view numThenAlphaPost = "f";
  const std::string ret = Lexer::parseInt(numThenAlpha);
  EXPECT_EQ("1234", ret);
  EXPECT_EQ(numThenAlphaPost, numThenAlpha);
}

TEST(LexerTest, ParseIntNumWithSpace) {
  std::string_view numWithSpace = "12 34";
  constexpr std::string_view numWithSpacePost;
  const std::string ret = Lexer::parseInt(numWithSpace);
  EXPECT_EQ("1234", ret);
  EXPECT_EQ(numWithSpacePost, numWithSpace);
}

TEST(LexerTest, ParseIntNumThenSpacesThenAlpha) {
  std::string_view numThenSpacesThenAlpha = "12 34 f";
  constexpr std::string_view numThenSpacesThenAlphaPost = "f";
  const std::string ret = Lexer::parseInt(numThenSpacesThenAlpha);
  EXPECT_EQ("1234", ret);
  EXPECT_EQ(numThenSpacesThenAlphaPost, numThenSpacesThenAlpha);
}

TEST(LexerTest, TokensFromString1) {
  TokenList tokens = Lexer::TokensFromInput("123 + 456");
  auto itr = tokens.begin();
  EXPECT_EQ("123",itr->literal); ++itr;
  EXPECT_EQ("+",itr->literal); ++itr;
  EXPECT_EQ("456",itr->literal); ++itr;
}

TEST(LexerTest, TokensFromString2) {
  TokenList tokens = Lexer::TokensFromInput("123 + 456");
  EXPECT_NO_THROW(  for (const auto& token : tokens) {
    auto toke = token.literal;
  });
}
TEST(LexerTest, TokensFromStringMemoryReleased) {
  TokenList::Iterator begin{nullptr};
  {
    TokenList tokens = Lexer::TokensFromInput("123 + 456");
    begin = tokens.begin();
  }
  EXPECT_NE(begin->literal,"123");
}
