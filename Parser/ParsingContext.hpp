//
// Created by Elijah on 10/29/2024.
//

#ifndef PARSINGCONTEXT_HPP
#define PARSINGCONTEXT_HPP

#include <deque>
#include "../Lexer/Lexer.cpp"

struct ParsingContext {
  Lexer& lexer;
  std::deque<Token> tokenQueue;
  Token currentToken;

  explicit ParsingContext(Lexer& lexer) : lexer(lexer) {
    currentToken = lexer.getNextToken();
  }

  void
  advance() {
    if (tokenQueue.empty()) {
      currentToken = lexer.getNextToken();
    } else {
      currentToken = tokenQueue.front();
      tokenQueue.pop_front();
    }
  }

  [[nodiscard]] Token
  peekNextToken() {
    tokenQueue.emplace_back(lexer.getNextToken());
    return tokenQueue.back();
  }
  [[nodiscard]] Token
  getLastTokenFromQueue() {
    return tokenQueue.back();
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
