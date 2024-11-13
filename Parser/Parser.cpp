// Created by Elijah on 8/13/2024.

#ifndef PARSER_CPP
#define PARSER_CPP

#include "Parser.hpp"

std::unique_ptr<Program> Parser::parseProgram() {
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

bool
Parser::IsFunctionDeclaration() {
  if (context.currentToken.kind != TokenKind::TK_IDENTIFIER || context.peekNextToken().kind != TokenKind::TK_COLONCOLON) {
    return false;
  }
  if (context.peekNextToken().raw_val == "static") {
    return true;
  }
  if (context.getLastToken().kind != TokenKind::TK_OPEN_PAREN) {
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

[[nodiscard]] bool
Parser::IsVariableDeclaration() {
  if (context.currentToken.kind != TokenKind::TK_IDENTIFIER) {
    return false;
  }
  if (context.getLastToken().raw_val!=":") {
    return false;
  }
  return IsTypeName(context.peekNextToken().raw_val);
}

bool
Parser::IsTypeName(const std::string& name) {
  static const std::set<std::string> typeNames {
    "char", "signed", "unsigned", "int", "long", "double", "void", "float", "struct"
  };
  return typeNames.contains(name);
}

[[nodiscard]] bool
Parser::IsStructDeclaration() const {
  return context.currentToken.raw_val == "struct";
}

#endif // PARSER_CPP
