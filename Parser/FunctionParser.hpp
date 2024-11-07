#ifndef FUNCTIONPARSER_HPP
#define FUNCTIONPARSER_HPP

#include "ParsingContext.hpp"
#include "ParserBase.hpp"
#include "StatementParser.hpp"

class FunctionParser : public ParserBase {
  StatementParser statementParser;
  public:
  explicit FunctionParser(ParsingContext& context) : ParserBase(context), statementParser(context) {}

  std::unique_ptr<FunctionDeclaration> ParseFunctionDeclaration() {
    auto returnType = ParseType();

    std::string name = context.currentToken.raw_val;
    context.advance();

    Expect("(");
    std::vector<std::unique_ptr<Parameter>> parameters;
    if (context.currentToken.raw_val != ")") {
      do {
        auto paramType = ParseType();
        std::string paramName = context.currentToken.raw_val;
        context.advance();
        parameters.push_back(std::make_unique<Parameter>(std::move(paramName), std::move(paramType)));
      } while (context.currentToken.raw_val == "," && (context.advance(), true));
    }
    Expect(")");

    std::optional<std::unique_ptr<Block>> body = std::nullopt;
    if (context.currentToken.raw_val == "{") {
      body = statementParser.ParseBlock();
    } else {
      Expect(";");
    }

    return std::make_unique<FunctionDeclaration>(
        std::move(name), std::move(parameters), std::move(body), std::move(returnType)
    );
  }

  private:
  std::unique_ptr<Type> ParseType() {
    return statementParser.ParseType();
  }
};

#endif // FUNCTIONPARSER_HPP
