//
// Created by Elijah Crain on 10/29/24.
//

#ifndef FUNCTIONPARSER_HPP
#define FUNCTIONPARSER_HPP

#include "ParsingContext.hpp"

class FunctionParser : public ParserBase {
  StatementParser statementParser;
  public:
  explicit FunctionParser(ParsingContext& context) : ParserBase(context), statementParser(context) {}

  std::unique_ptr<FunctionDef>
  ParseFunction() {
    std::string name = context.currentToken->raw_val;
    context.advance();

    Expect("::");
    Expect("(");
    Expect(")");
    std::string returnType = context.currentToken->raw_val;
    context.advance();
    Expect("{");

    int allocationSize = 0;
    std::vector<std::unique_ptr<Statement>> statements;
    while (context.currentToken && context.currentToken->raw_val != "}") {
      auto statement = statementParser.ParseStatement();
      if (auto declare = dynamic_cast<Declare*>(statement.get())) {
        if (declare->type == "int") {
          allocationSize += 16; // Assuming int is 16 bytes as per original code
        }
      }
      statements.push_back(std::move(statement));
    }
    Expect("}");
    return std::make_unique<FunctionDef>(std::move(name), allocationSize,std::move(returnType), std::move(statements));
  }
};

#endif //FUNCTIONPARSER_HPP
