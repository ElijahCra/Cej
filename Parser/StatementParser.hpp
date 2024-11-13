//
// Created by Elijah Crain on 10/29/24.
//

#ifndef STATEMENTPARSER_HPP
#define STATEMENTPARSER_HPP

#include "ParsingContext.hpp"
#include "ParserBase.hpp"
#include "ExpressionParser.cpp"

class StatementParser : public ParserBase {
  ExpressionParser expressionParser;
  public:
  explicit StatementParser(ParsingContext& context) : ParserBase(context), expressionParser(context) {}

  std::unique_ptr<Statement>
  ParseStatement();

  std::unique_ptr<Declaration>
  ParseVariableDeclaration();

  std::unique_ptr<Block>
  ParseBlock();

  std::unique_ptr<Declaration>
  ParseStructDeclaration();

  std::unique_ptr<Type>
  ParseType();

  static bool
  IsTypeName(const std::string& name);


  private:

  [[nodiscard]] bool IsDeclaration() const;
  static bool IsPrimitiveType(const std::string& typeName);

  static PrimitiveType StringToPrimitiveType(const std::string& typeName);

  std::unique_ptr<MemberDeclaration> ParseMemberDeclaration();


  std::unique_ptr<Statement> ParseIfStatement(){return nullptr;};//todo
  std::unique_ptr<Statement> ParseWhileStatement(){return nullptr;};//todo
  std::unique_ptr<Statement> ParseForStatement(){return nullptr;};//todo
};

#endif // STATEMENTPARSER_HPP
