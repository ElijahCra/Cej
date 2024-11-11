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
  static bool IsPrimitiveType(const std::string& typeName) {
    static const std::set<std::string> primitiveTypes = {
        "char", "int", "long", "double", "void", "float"
    };
    return primitiveTypes.contains(typeName);
  }

  PrimitiveType StringToPrimitiveType(const std::string& typeName) {
    if (typeName == "char") return PrimitiveType::Char;
    if (typeName == "int") return PrimitiveType::Int;
    if (typeName == "long") return PrimitiveType::Long;
    if (typeName == "double") return PrimitiveType::Double;
    if (typeName == "float") return PrimitiveType::Double;
    if (typeName == "void") return PrimitiveType::Void;
    throw std::runtime_error("Unknown primitive type: " + typeName);
  }

    std::unique_ptr<MemberDeclaration> ParseMemberDeclaration() {
      auto memberType = ParseType();
      std::string memberName = context.currentToken.raw_val;
      context.advance();
      Expect(";");
      return std::make_unique<MemberDeclaration>(std::move(memberName), std::move(memberType));
    }

    bool IsDeclaration() {
        return IsTypeName(context.currentToken.raw_val);
    }

    // Placeholder methods for parsing statements not fully implemented
    std::unique_ptr<Statement> ParseIfStatement() { /* Implement accordingly */ }
    std::unique_ptr<Statement> ParseWhileStatement() { /* Implement accordingly */ }
    std::unique_ptr<Statement> ParseForStatement() { /* Implement accordingly */ }
};

#endif // STATEMENTPARSER_HPP
