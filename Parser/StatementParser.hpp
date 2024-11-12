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
  IsTypeName(const std::string& name) {
        return IsPrimitiveType(name) || name == "struct";
    }

private:
  static bool IsPrimitiveType(const std::string& typeName);

  static PrimitiveType StringToPrimitiveType(const std::string& typeName);

  std::unique_ptr<MemberDeclaration> ParseMemberDeclaration();

  bool IsDeclaration();

    // Placeholder methods for parsing statements not fully implemented
    std::unique_ptr<Statement> ParseIfStatement() { /* Implement accordingly */ }
    std::unique_ptr<Statement> ParseWhileStatement() { /* Implement accordingly */ }
    std::unique_ptr<Statement> ParseForStatement() { /* Implement accordingly */ }
};

#endif // STATEMENTPARSER_HPP
