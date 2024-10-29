//
// Created by Elijah on 10/29/2024.
//

#ifndef PARSINGCONTEXT_HPP
#define PARSINGCONTEXT_HPP

#include "../Lexer/Lexer.cpp"

struct ParsingContext {
  Lexer& lexer;
  std::optional<Token> currentToken;

  explicit ParsingContext(Lexer& lexer) : lexer(lexer) {
    currentToken = lexer.getNextToken();
  }

  void
  advance() {
    currentToken = lexer.getNextToken();
  }

  [[nodiscard]] int
  getCurrentLine() const {
    return lexer.getCurrentLine();
  }
  [[nodiscard]] int
  getCurrentPosition() const {
    return lexer.getCurrentPosition();
  }
};

#endif //PARSINGCONTEXT_HPP
