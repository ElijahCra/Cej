//
// Created by Elijah Crain on 11/12/24.
//

#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <array>
#include "../Lexer/Lexer.cpp"
#include "ParserTypes.hpp"
#include "StatementParser.cpp"
#include "FunctionParser.hpp"
#include "ParsingContext.hpp"

class Parser {
  public:
  explicit Parser(Lexer& lexer) : context(lexer), statementParser(context), functionParser(context) {}

  std::unique_ptr<Program> parseProgram();

  [[nodiscard]] bool IsStructDeclaration() const;

  private:
  ParsingContext context;
  StatementParser statementParser;
  FunctionParser functionParser;

  [[nodiscard]] bool isDeclaration();
  [[nodiscard]] bool IsFunctionDeclaration();
  [[nodiscard]] bool IsVariableDeclaration();
  static bool IsTypeName(const std::string& name);
};

#endif //PARSER_HPP
