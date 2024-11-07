#ifndef EXPRESSIONPARSER_HPP
#define EXPRESSIONPARSER_HPP

#include "ParserBase.hpp"
#include "ParsingContext.hpp"
#include "ParserTypes.hpp"

class ExpressionParser : public ParserBase {
public:
    explicit ExpressionParser(ParsingContext& context) : ParserBase(context) {}

    std::unique_ptr<Exp> ParseExpression() {
        return ParseAssignmentExpression();
    }

private:
    std::unique_ptr<Exp> ParseAssignmentExpression() {
        auto lhs = ParseConditionalExpression();
        if (context.currentToken.raw_val == "=") {
            context.advance();
            auto rhs = ParseAssignmentExpression();
            return std::make_unique<Assignment>(std::move(lhs), std::move(rhs));
        }
        return lhs;
    }

    std::unique_ptr<Exp> ParseConditionalExpression() {
        auto condition = ParseLogicalOrExpression();
        if (context.currentToken.raw_val == "?") {
            context.advance();
            auto trueExp = ParseExpression();
            Expect(":");
            auto falseExp = ParseConditionalExpression();
            return std::make_unique<Conditional>(std::move(condition), std::move(trueExp), std::move(falseExp));
        }
        return condition;
    }

    // Implement the rest of the parsing methods (ParseLogicalOrExpression, ParseLogicalAndExpression, etc.)

    std::unique_ptr<Exp> ParsePrimaryExpression() {
        if (context.currentToken.kind == TokenKind::TK_INT) {
            int value = std::stoi(context.currentToken.raw_val);
            context.advance();
            return std::make_unique<Constant>(value);
        }
        if (context.currentToken.kind == TokenKind::TK_IDENTIFIER) {
            std::string name = context.currentToken.raw_val;
            context.advance();
            return std::make_unique<Var>(std::move(name));
        }
        if (context.currentToken.raw_val == "(") {
            context.advance();
            auto exp = ParseExpression();
            Expect(")");
            return exp;
        }
        if (context.currentToken.kind == TokenKind::TK_STRING_LITERAL) {
            std::string value = context.currentToken.raw_val;
            context.advance();
            return std::make_unique<StringLiteral>(std::move(value));
        }
        throw std::runtime_error("Unexpected token in primary expression at line: " + std::to_string(context.getCurrentLine()));
    }

    bool IsTypeName(const std::string& name) {
        return statementParser.IsTypeName(name);
    }

    StatementParser statementParser{context};
};

#endif // EXPRESSIONPARSER_HPP
