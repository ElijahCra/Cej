// Created by Elijah on 8/13/2024.

#ifndef LEXER_CPP
#define LEXER_CPP

#include <string>
#include <string_view>
#include <stdexcept>
#include <array>
#include <fstream>
#include <iostream>
#include <optional>
#include "Token.hpp"



class Lexer {
  public:
  static constexpr std::array<std::string_view,3> keyWords = {"return", "ret","int"};

  explicit Lexer(const std::string& path) : path(path) {
    std::ifstream file(path);
    if (!file.is_open()) {
      std::cerr << "Failed to open file." << std::endl;
      throw std::runtime_error("Failed to open file.");
    }
    input=std::move(file);
  }

  ~Lexer() {
    if (input.is_open()) {
      input.close();
    }
    std::cout << std::format("Processed {} lines in lexer for file {}\n ",lineCount,path);
  }
  Lexer(const Lexer&) = delete;
  Lexer& operator=(const Lexer&) = delete;

  std::optional<Token> getNextToken() {
    char ch;
    while (input.get(ch)) {
      if('\n' == ch) {
        lineCount += 1;
        continue;
      }
      if (std::isspace(ch)) {
        continue;
      }
      if (std::isdigit(ch)) {
        return makeTokenFromInt(ch);
      }
      if (std::isalpha(ch)) {
        return makeTokenFromText(ch);
      }
      if (std::ispunct(ch)) {
        return makeTokenFromPunctuation(ch);
      }
      throw std::runtime_error("Unexpected character: " + std::string(1, ch));
    }
    return Token{TokenKind::TK_EOF, ""};
  }

  std::string getCurrentLine() {
    return std::format("{}",lineCount);
  }

  private:
  std::ifstream input;
  int lineCount=1;
  const std::string path;

  Token makeTokenFromInt(char firstChar) {
    std::string value(1, firstChar);
    char ch;
    while (input.get(ch) && (std::isdigit(ch) || std::isspace(ch))) {
      if (!std::isspace(ch)) {
        value += ch;
      }
    }
    if (input) {
      input.putback(ch);
    }
    return {TokenKind::TK_INT, value};
  }

  Token makeTokenFromText(char firstChar) {
    std::string value(1, firstChar);
    char ch;
    while (input.get(ch) && (std::isalnum(ch) || ch == '_')) {
      value += ch;
    }
    if (input) {
      input.putback(ch);
    }
    if (std::ranges::find(keyWords, value) != keyWords.end()) {
      return {TokenKind::TK_KEYWORD, value};
    }
    return {TokenKind::TK_IDENTIFIER, value};
  }

  Token makeTokenFromPunctuation(char firstChar) {
    using enum TokenKind;
    switch (firstChar) {
      case '+': return {TK_PLUS, "+"};
      case '-': return {TK_MINUS, "-"};
      case '/': return {TK_SLASH, "/"};
      case '*': return {TK_ASTERISK, "*"};
      case ':': {
        char next;
        if (input.get(next)) {
          if (next == ':') return {TK_COLONCOLON, "::"};
          if (next == '=') return {TK_COLONEQUAL, ":="};
          input.putback(next);
        }
        return {TK_COLON, ":"};
      }
      case '=': return {TK_EQUAL, "="};
      case ';': return {TK_SEMICOLON, ";"};
      case '{': return {TK_OPEN_BRACE, "{"};
      case '}': return {TK_CLOSE_BRACE, "}"};
      case '(': return {TK_OPEN_PAREN, "("};
      case ')': return {TK_CLOSE_PAREN, ")"};
      case ',': return {TK_COMMA, ","};
      default: throw std::runtime_error("Invalid punctuation");
    }
  }
};

#endif // LEXER_CPP