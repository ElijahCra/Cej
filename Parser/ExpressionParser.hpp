#ifndef EXPRESSIONPARSER_HPP
#define EXPRESSIONPARSER_HPP

#include "ParserBase.hpp"
#include "ParsingContext.hpp"

class ExpressionParser : public ParserBase {
public:
    explicit ExpressionParser(ParsingContext& context) : ParserBase(context) {}

    std::unique_ptr<Exp> parseExpression() {
        return parseAssignmentExpression();
    }

private:
    std::unique_ptr<Exp> parseAssignmentExpression() {
        auto lhs = parseConditionalExpression();
        if (context.currentToken.raw_val == "=") {
            context.advance();
            auto rhs = parseAssignmentExpression();
            return std::make_unique<Assignment>(std::move(lhs), std::move(rhs));
        } else {
            return lhs;
        }
    }

    std::unique_ptr<Exp> parseConditionalExpression() {
        auto condition = parseLogicalOrExpression();
        if (context.currentToken.raw_val == "?") {
            context.advance();
            auto trueExp = parseExpression();
            Expect(":");
            auto falseExp = parseConditionalExpression();
            return std::make_unique<Conditional>(std::move(condition), std::move(trueExp), std::move(falseExp));
        } else {
            return condition;
        }
    }

    std::unique_ptr<Exp> parseLogicalOrExpression() {
        auto lhs = parseLogicalAndExpression();
        while (context.currentToken.raw_val == "||") {
            context.advance();
            auto rhs = parseLogicalAndExpression();
            lhs = std::make_unique<BinOp>(BinaryOperator::Or, std::move(lhs), std::move(rhs));
        }
        return lhs;
    }

    std::unique_ptr<Exp> parseLogicalAndExpression() {
        auto lhs = parseEqualityExpression();
        while (context.currentToken.raw_val == "&&") {
            context.advance();
            auto rhs = parseEqualityExpression();
            lhs = std::make_unique<BinOp>(BinaryOperator::And, std::move(lhs), std::move(rhs));
        }
        return lhs;
    }

    std::unique_ptr<Exp> parseEqualityExpression() {
        auto lhs = parseRelationalExpression();
        while (context.currentToken.raw_val == "==" || context.currentToken.raw_val == "!=") {
            BinaryOperator op = (context.currentToken.raw_val == "==") ? BinaryOperator::Equal : BinaryOperator::NotEqual;
            context.advance();
            auto rhs = parseRelationalExpression();
            lhs = std::make_unique<BinOp>(op, std::move(lhs), std::move(rhs));
        }
        return lhs;
    }

    std::unique_ptr<Exp> parseRelationalExpression() {
        auto lhs = parseAdditiveExpression();
        while (context.currentToken.raw_val == "<" || context.currentToken.raw_val == ">" ||
               context.currentToken.raw_val == "<=" || context.currentToken.raw_val == ">=") {
            BinaryOperator op;
            if (context.currentToken.raw_val == "<") op = BinaryOperator::LessThan;
            else if (context.currentToken.raw_val == ">") op = BinaryOperator::GreaterThan;
            else if (context.currentToken.raw_val == "<=") op = BinaryOperator::LessOrEqual;
            else op = BinaryOperator::GreaterOrEqual;

            context.advance();
            auto rhs = parseAdditiveExpression();
            lhs = std::make_unique<BinOp>(op, std::move(lhs), std::move(rhs));
        }
        return lhs;
    }

    std::unique_ptr<Exp> parseAdditiveExpression() {
        auto lhs = parseMultiplicativeExpression();
        while (context.currentToken.raw_val == "+" || context.currentToken.raw_val == "-") {
            BinaryOperator op = (context.currentToken.raw_val == "+") ? BinaryOperator::Add : BinaryOperator::Subtract;
            context.advance();
            auto rhs = parseMultiplicativeExpression();
            lhs = std::make_unique<BinOp>(op, std::move(lhs), std::move(rhs));
        }
        return lhs;
    }

    std::unique_ptr<Exp> parseMultiplicativeExpression() {
        auto lhs = parseUnaryExpression();
        while (context.currentToken.raw_val == "*" || context.currentToken.raw_val == "/" || context.currentToken.raw_val == "%") {
            BinaryOperator op;
            if (context.currentToken.raw_val == "*") op = BinaryOperator::Multiply;
            else if (context.currentToken.raw_val == "/") op = BinaryOperator::Divide;
            else op = BinaryOperator::Remainder;

            context.advance();
            auto rhs = parseUnaryExpression();
            lhs = std::make_unique<BinOp>(op, std::move(lhs), std::move(rhs));
        }
        return lhs;
    }

    std::unique_ptr<Exp> parseUnaryExpression() {
        if (context.currentToken.raw_val == "+" || context.currentToken.raw_val == "-" ||
            context.currentToken.raw_val == "~" || context.currentToken.raw_val == "!") {
            UnaryOperator op;
            if (context.currentToken.raw_val == "+") {
                context.advance();
                return parseUnaryExpression();
            } else if (context.currentToken.raw_val == "-") {
                op = UnaryOperator::Negate;
            } else if (context.currentToken.raw_val == "~") {
                op = UnaryOperator::Complement;
            } else {
                op = UnaryOperator::Not;
            }
            context.advance();
            auto operand = parseUnaryExpression();
            return std::make_unique<UnOp>(op, std::move(operand));
        } else if (context.currentToken.raw_val == "(") {
            size_t savedPosition = context.getCurrentPosition();
            auto savedToken = context.currentToken;

            context.advance();
            auto type = typeParser.parseType();
            if (type && context.currentToken.raw_val == ")") {
                context.advance();
                auto operand = parseUnaryExpression();
                return std::make_unique<Cast>(std::move(type), std::move(operand));
            } else {
                context.setPosition(savedPosition);
                context.currentToken = savedToken;
            }
        }
        return parsePostfixExpression();
    }

    std::unique_ptr<Exp> parsePostfixExpression() {
        auto expr = parsePrimaryExpression();
        while (true) {
            if (context.currentToken.raw_val == "[") {
                context.advance();
                auto index = parseExpression();
                Expect("]");
                expr = std::make_unique<Subscript>(std::move(expr), std::move(index));
            } else if (context.currentToken.raw_val == "(") {
                expr = parseFunctionCall(std::move(expr));
            } else if (context.currentToken.raw_val == ".") {
                context.advance();
                if (context.currentToken.kind != TokenKind::TK_IDENTIFIER) {
                    throw std::runtime_error("Expected identifier after '.' at line: " + std::to_string(context.getCurrentLine()));
                }
                std::string member = context.currentToken.raw_val;
                context.advance();
                expr = std::make_unique<Dot>(std::move(expr), std::move(member));
            } else if (context.currentToken.raw_val == "->") {
                context.advance();
                if (context.currentToken.kind != TokenKind::TK_IDENTIFIER) {
                    throw std::runtime_error("Expected identifier after '->' at line: " + std::to_string(context.getCurrentLine()));
                }
                std::string member = context.currentToken.raw_val;
                context.advance();
                expr = std::make_unique<Arrow>(std::move(expr), std::move(member));
            } else {
                break;
            }
        }
        return expr;
    }

    std::unique_ptr<Exp> parsePrimaryExpression() {
        if (context.currentToken.kind == TokenKind::TK_INT ||
            context.currentToken.kind == TokenKind::TK_CHAR ||
            context.currentToken.kind == TokenKind::TK_DOUBLE) {
            auto constant = parseConstant();
            return std::move(constant);
        } else if (context.currentToken.kind == TokenKind::TK_STRING) {
            std::string value = context.currentToken.raw_val;
            context.advance();
            return std::make_unique<StringLiteral>(std::move(value));
        } else if (context.currentToken.kind == TokenKind::TK_IDENTIFIER) {
            std::string name = context.currentToken.raw_val;
            context.advance();
            return std::make_unique<Var>(std::move(name));
        } else if (context.currentToken.raw_val == "(") {
            context.advance();
            auto expr = parseExpression();
            Expect(")");
            return expr;
        } else {
            throw std::runtime_error("Unexpected token in primary expression at line: " + std::to_string(context.getCurrentLine()));
        }
    }

    std::unique_ptr<Exp> parseFunctionCall(std::unique_ptr<Exp> func) {
        Expect("(");
        std::vector<std::unique_ptr<Exp>> arguments;
        if (context.currentToken.raw_val != ")") {
            do {
                arguments.push_back(parseAssignmentExpression());
            } while (context.currentToken.raw_val == "," && (context.advance(), true));
        }
        Expect(")");
        if (auto var = dynamic_cast<Var*>(func.get())) {
            return std::make_unique<FunctionCall>(std::move(var->name), std::move(arguments));
        } else {
            throw std::runtime_error("Function calls on expressions not supported yet at line: " + std::to_string(context.getCurrentLine()));
        }
    }

    std::unique_ptr<Exp> parseConstant() {
        if (context.currentToken.kind == TokenKind::TK_INT) {
            int value = std::stoi(context.currentToken.raw_val);
            context.advance();
            return std::make_unique<Constant>(value);
        } else if (context.currentToken.kind == TokenKind::TK_CHAR) {
            char value = context.currentToken.raw_val[0];
            context.advance();
            return std::make_unique<Constant>(value);
        } else if (context.currentToken.kind == TokenKind::TK_DOUBLE) {
            double value = std::stod(context.currentToken.raw_val);
            context.advance();
            return std::make_unique<Constant>(value);
        } else {
            throw std::runtime_error("Unknown constant type at line: " + std::to_string(context.getCurrentLine()));
        }
    }
};

#endif // EXPRESSIONPARSER_HPP
