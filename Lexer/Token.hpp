//
// Created by Elijah on 10/25/2024.
//
#ifndef TOKEN_HPP
#define TOKEN_HPP

enum class TokenKind {
  TK_EOF,
  TK_INT,
  TK_KEYWORD,
  TK_IDENTIFIER,
  TK_PLUS,
  TK_MINUS,
  TK_SLASH,
  TK_ASTERISK,
  TK_COLON,
  TK_COLONCOLON,
  TK_EQUAL,
  TK_COLONEQUAL,
  TK_SEMICOLON,
  TK_OPEN_BRACE,
  TK_CLOSE_BRACE,
  TK_OPEN_PAREN,
  TK_CLOSE_PAREN,
  TK_COMMA,
  TK_DOUBLE_QUOTE
};

struct Token {
  TokenKind kind{};
  std::string raw_val;
};

#endif //TOKEN_HPP