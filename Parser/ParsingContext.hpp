//
// Created by Elijah on 10/29/2024.
//

// ParsingContext.hpp

#ifndef PARSINGCONTEXT_HPP
#define PARSINGCONTEXT_HPP

#include "../Lexer/Lexer.cpp"

struct ParsingContext {
  Lexer& lexer;
  std::optional<Token> currentToken;
  std::vector<std::string> namespaceStack;

  explicit ParsingContext(Lexer& lexer) : lexer(lexer) {
    currentToken = lexer.getNextToken();
  }

  void advance() {
    currentToken = lexer.getNextToken();
  }

  [[nodiscard]] std::optional<Token> checkForDoubleColon() const {
    return lexer.peekForDoubleColon();
  }

  [[nodiscard]] int getCurrentLine() const {
    return lexer.getCurrentLine();
  }

  [[nodiscard]] int getCurrentPosition() const {
    return lexer.getCurrentPosition();
  }

  void enterNamespace(const std::string& ns) {
    namespaceStack.push_back(ns);
  }

  void exitNamespace() {
    if (!namespaceStack.empty()) {
      namespaceStack.pop_back();
    } else {
      throw std::runtime_error("No namespace to exit");
    }
  }

  std::string getCurrentNamespace() const {
    std::string fullNamespace;
    for (const auto& ns : namespaceStack) {
      if (!fullNamespace.empty()) {
        fullNamespace += "::";
      }
      fullNamespace += ns;
    }
    return fullNamespace;
  }
};

#endif //PARSINGCONTEXT_HPP
