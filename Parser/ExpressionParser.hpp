#ifndef EXPRESSIONPARSER_HPP
#define EXPRESSIONPARSER_HPP

#include "ParserBase.hpp"
#include "ParsingContext.hpp"
#include "ParserTypes.hpp"

class ExpressionParser : public ParserBase {
    public:
    explicit ExpressionParser(ParsingContext& context) : ParserBase(context) {}

    std::unique_ptr<Exp> ParseExpression();

    private:
    std::unique_ptr<Exp> ParseAssignmentExpression();
    std::unique_ptr<Exp> ParseLogicalOrExpression();
    std::unique_ptr<Exp> ParseLogicalAndExpression();
    std::unique_ptr<Exp> ParseEqualityExpression();
    std::unique_ptr<Exp> ParseRelationalExpression();
    std::unique_ptr<Exp> ParseAdditiveExpression();
    std::unique_ptr<Exp> ParseMultiplicativeExpression();
    std::unique_ptr<Exp> ParseUnaryExpression();
    std::unique_ptr<Exp> ParsePrimaryExpression();
    std::unique_ptr<Exp> ParseFunctionCall(std::string name);

    [[nodiscard]] bool IsTypeName(const std::string& name) const;
};

#endif // EXPRESSIONPARSER_HPP
