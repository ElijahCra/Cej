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
    std::vector<std::unique_ptr<ASTNode>> nodes;
    while (context.currentToken.kind != TokenKind::TK_EOF) {
      if (IsFunctionDefinition()) {
        nodes.push_back(functionParser.ParseFunction());
      } else {
        nodes.push_back(statementParser.ParseStatement());
      }
    }
    return std::make_unique<CompilationUnit>(std::move(nodes));
  }

  private:
  ParsingContext context;
  StatementParser statementParser;
  FunctionParser functionParser;

  bool IsFunctionDefinition() {
    if (context.currentToken.kind != TokenKind::TK_IDENTIFIER || context.addNextTokenToDequeue().kind != TokenKind::TK_COLONCOLON) {
      return false;
    }
    if (context.addNextTokenToDequeue().raw_val == "static") {
      return true;
    }
    if (context.getLastTokenFromQueue().kind != TokenKind::TK_OPEN_PAREN) {
      return false;
    }
    if (context.addNextTokenToDequeue().kind == TokenKind::TK_CLOSE_PAREN) {
      return true;
    }
    if (context.addNextTokenToDequeue().kind == TokenKind::TK_IDENTIFIER && context.addNextTokenToDequeue().kind == TokenKind::TK_COLON) {
      return true;
    }
    return false;
  }

};

#endif // PARSER_CPP
