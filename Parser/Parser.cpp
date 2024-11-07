// Created by Elijah on 8/13/2024.

#ifndef PARSER_CPP
#define PARSER_CPP

#include <vector>
#include <set>
#include "../Lexer/Lexer.cpp"
#include "ParserTypes.hpp"
#include "StatementParser.hpp"
#include "FunctionParser.hpp"
#include "ParsingContext.hpp"

class Parser {
  public:
  explicit Parser(Lexer& lexer) : context(lexer), statementParser(context), functionParser(context) {}

  std::unique_ptr<Program> parseProgram() {
    std::vector<std::unique_ptr<Declaration>> declarations;
    while (context.currentToken.kind != TokenKind::TK_EOF) {
      if (IsFunctionDeclaration()) {
        declarations.push_back(functionParser.ParseFunctionDeclaration());
      } else if (IsVariableDeclaration()) {
        declarations.push_back(statementParser.ParseVariableDeclaration());
      } else if (IsStructDeclaration()) {
        declarations.push_back(statementParser.ParseStructDeclaration());
      } else {
        throw std::runtime_error("Unexpected declaration at line: " + std::to_string(context.getCurrentLine()));
      }
    }
    return std::make_unique<Program>(std::move(declarations));
  }

  private:
  ParsingContext context;
  StatementParser statementParser;
  FunctionParser functionParser;

  bool
  IsFunctionDeclaration() {
    if (context.currentToken.kind != TokenKind::TK_IDENTIFIER || context.peekNextToken().kind != TokenKind::TK_COLONCOLON) {
      return false;
    }
    if (context.peekNextToken().raw_val == "static") {
      return true;
    }
    if (context.getLastTokenFromQueue().kind != TokenKind::TK_OPEN_PAREN) {
      return false;
    }
    if (context.peekNextToken().kind == TokenKind::TK_CLOSE_PAREN) {
      return true;
    }
    if (context.peekNextToken().kind == TokenKind::TK_IDENTIFIER && context.peekNextToken().kind == TokenKind::TK_COLON) {
      return true;
    }
    return false;
  }

  bool
  IsVariableDeclaration() const {
    ParsingContext tempContext = context;
    if (!IsTypeName(tempContext.currentToken.raw_val)) {
      return false;
    }
    tempContext.advance();

    if (tempContext.currentToken.kind == TokenKind::TK_IDENTIFIER) {
      return true;
    }

    return false;
  }

  [[nodiscard]] bool
  IsStructDeclaration() const {
    return context.currentToken.raw_val == "struct";
  }

  static bool
  IsTypeName(const std::string& name) {
    static constexpr std::set<std::string> typeNames {
      "char", "signed", "unsigned", "int", "long", "double", "void", "float", "struct"
    };
    return typeNames.contains(name);
  }
};

#endif // PARSER_CPP
