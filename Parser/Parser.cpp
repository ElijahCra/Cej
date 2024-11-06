// Created by Elijah on 8/13/2024.

#ifndef PARSER_CPP
#define PARSER_CPP

#include <optional>
#include <vector>
#include "../Lexer/Lexer.cpp"
#include "ParserTypes.hpp"
#include "DeclarationParser.cpp"
#include "ParsingContext.hpp"

class Parser {
  public:
  explicit Parser(Lexer& lexer)
      : context(lexer), declarationParser(context) {}

  std::unique_ptr<Program> parseProgram() {
    std::vector<std::unique_ptr<Declaration>> declarations;
    while (context.currentToken.kind != TokenKind::TK_EOF) {
      auto decl = declarationParser.parseDeclaration();
      if (decl) {
        declarations.push_back(std::move(decl));
      } else {
        throw std::runtime_error("Failed to parse declaration at line: " + std::to_string(context.getCurrentLine()));
      }
    }
    return std::make_unique<Program>(std::move(declarations));
  }

  private:
  ParsingContext context;
  DeclarationParser declarationParser;
};

#endif // PARSER_CPP