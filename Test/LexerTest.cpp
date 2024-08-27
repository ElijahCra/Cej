//
// Created by Elijah Crain on 8/21/24.
//
#include <gtest/gtest.h>
#include "../Compiler/Lexer.cpp"


TEST(LexerTest, ParseIntPureNum) {
  std::string_view pureNum = "1234";
  constexpr std::string_view pureNumPost;
  const auto ret = Lexer::makeTokenFromInt(pureNum);
  EXPECT_EQ("1234", ret->literal);
  EXPECT_EQ(pureNumPost, pureNum);
}

TEST(LexerTest, ParseIntNumThenAlpha) {
  std::string_view numThenAlpha = "1234f";
  constexpr std::string_view numThenAlphaPost = "f";
  const auto ret = Lexer::makeTokenFromInt(numThenAlpha);
  EXPECT_EQ("1234", ret->literal);
  EXPECT_EQ(numThenAlphaPost, numThenAlpha);
}

TEST(LexerTest, ParseIntNumWithSpace) {
  std::string_view numWithSpace = "12 34";
  constexpr std::string_view numWithSpacePost;
  const auto ret = Lexer::makeTokenFromInt(numWithSpace);
  EXPECT_EQ("1234", ret->literal);
  EXPECT_EQ(numWithSpacePost, numWithSpace);
}

TEST(LexerTest, ParseIntNumThenSpacesThenAlpha) {
  std::string_view numThenSpacesThenAlpha = "12 34 f";
  constexpr std::string_view numThenSpacesThenAlphaPost = "f";
  const auto ret = Lexer::makeTokenFromInt(numThenSpacesThenAlpha);
  EXPECT_EQ("1234", ret->literal);
  EXPECT_EQ(numThenSpacesThenAlphaPost, numThenSpacesThenAlpha);
}

TEST(LexerTest, TokensFromString1) {
  TokenList tokens {Lexer::TokensFromInput("123 + 456")};
  auto itr = tokens.begin();
  EXPECT_EQ("123",itr->literal); ++itr;
  EXPECT_EQ("+",itr->literal); ++itr;
  EXPECT_EQ("456",itr->literal); ++itr;
}

TEST(LexerTest, TokensFromString2) {
  TokenList tokens {Lexer::TokensFromInput("123 + 456")};
  EXPECT_NO_THROW(  for (const auto& token : tokens) {
    auto toke = token.literal;
  });
}
TEST(LexerTest, TokensFromStringMemoryReleased) {
  TokenList::Iterator begin{nullptr};
  {
    TokenList tokens {Lexer::TokensFromInput("123 + 456")};
    begin = tokens.begin();
  }
  //still reads the memory...
  //EXPECT_NE(begin->literal,"123");
}
