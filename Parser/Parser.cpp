// Created by Elijah on 8/13/2024.

#// Parser.cpp

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
    auto tokenIter = context.currentToken;
    if (!tokenIter || tokenIter->kind != TokenKind::TK_IDENTIFIER) {
      return false;
    }

    // Simulate parsing namespace-qualified name
    while (tokenIter && tokenIter->kind == TokenKind::TK_IDENTIFIER) {
      tokenIter = context.peekNextToken();
      if (tokenIter && tokenIter->raw_val == "::") {
        lexer.consumeToken();
        tokenIter = lexer.peekNextToken();
        if (!tokenIter || tokenIter->kind != TokenKind::TK_IDENTIFIER) {
          return false;
        }
        lexer.consumeToken();
      } else {
        break;
      }
    }

    // After parsing name, check for '('
    if (tokenIter && tokenIter->raw_val == "(") {
      return true;
    }
    return false;
  }
};

#endif // PARSER_CPP
