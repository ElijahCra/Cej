//
// Created by Elijah Crain on 8/21/24.
//
#include <gtest/gtest.h>
#include "../Compiler/Lexer.cpp"


TEST(LexerTest, ParseIntPureNum) {
  std::string_view pureNum = "1234";
  std::string_view pureNumPost = "";
  std::string ret = Lexer::parseInt(pureNum);
  EXPECT_EQ("1234", ret);
  EXPECT_EQ(pureNumPost, pureNum);
}

TEST(LexerTest, ParseIntNumThenAlpha) {
  std::string_view numThenAlpha = "1234f";
  std::string_view numThenAlphaPost = "f";
  std::string ret = Lexer::parseInt(numThenAlpha);
  EXPECT_EQ("1234", ret);
  EXPECT_EQ(numThenAlphaPost, numThenAlpha);
}

TEST(LexerTest, ParseIntNumWithSpace) {
  std::string_view numWithSpace = "12 34";
  std::string_view numWithSpacePost = "";
  std::string ret = Lexer::parseInt(numWithSpace);
  EXPECT_EQ("1234", ret);
  EXPECT_EQ(numWithSpacePost, numWithSpace);
}

TEST(LexerTest, ParseIntNumThenSpacesThenAlpha) {
  std::string_view numThenSpacesThenAlpha = "12 34 f";
  std::string_view numThenSpacesThenAlphaPost = "f";
  std::string ret = Lexer::parseInt(numThenSpacesThenAlpha);
  EXPECT_EQ("1234", ret);
  EXPECT_EQ(numThenSpacesThenAlphaPost, numThenSpacesThenAlpha);
}
