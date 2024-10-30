// Created by Elijah on 8/13/2024.

#ifndef PARSER_CPP
#define PARSER_CPP

#include <optional>
#include <vector>
#include "../Lexer/Lexer.cpp"
#include "ParserTypes.hpp"
#include "StatementParser.hpp"
#include "FunctionParser.hpp"
#include "ParsingContext.hpp"

class Parser {
  public:
  explicit Parser(Lexer& lexer) : context(lexer), statementParser(context), functionParser(context) {}

  std::unique_ptr<CompilationUnit> parseUnit() {
    std::vector<std::unique_ptr<Statement>> statements;
    while (context.currentToken && context.currentToken->kind != TokenKind::TK_EOF) {
      if (IsFunctionDefinition()) {
        statements.push_back(functionParser.ParseFunction());
      } else {
        statements.push_back(statementParser.ParseStatement());
      }
    }
    return std::make_unique<CompilationUnit>(std::move(statements));
  }

  private:
  ParsingContext context;
  StatementParser statementParser;
  FunctionParser functionParser;

  bool IsFunctionDefinition() {
    // Implement logic to determine if the current token sequence represents a function definition
    // For simplicity, let's assume if we see an identifier followed by '::', it's a function
    return context.currentToken && context.currentToken->kind == TokenKind::TK_IDENTIFIER && context.checkForDoubleColon().has_value();
  }

};

#endif // PARSER_CPP
