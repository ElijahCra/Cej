//
// Created by Elijah on 10/29/2024.
//

#ifndef EXPRESSIONPARSER_HPP
#define EXPRESSIONPARSER_HPP

#include "ParserBase.hpp"

class ExpressionParser : public ParserBase {
public:
    std::unique_ptr<Exp> ParseExpression() {
        return ParseAdditiveExpression();
    }

private:
    std::unique_ptr<Exp> ParseAdditiveExpression() {
        auto left = ParseMultiplicativeExpression();
        while (currentToken && (currentToken->raw_val == "+" || currentToken->raw_val == "-")) {
            BinaryOperator op = (currentToken->raw_val == "+") ? BinaryOperator::Add : BinaryOperator::Sub;
            context.advance();
            auto right = ParseMultiplicativeExpression();
            left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
        }
        return left;
    }

    std::unique_ptr<Exp> ParseMultiplicativeExpression() {
        auto left = ParseUnaryExpression();
        while (currentToken && (currentToken->raw_val == "*" || currentToken->raw_val == "/")) {
            BinaryOperator op = (currentToken->raw_val == "*") ? BinaryOperator::Mul : BinaryOperator::Div;
            advance();
            auto right = ParseUnaryExpression();
            left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
        }
        return left;
    }

    std::unique_ptr<Exp> ParseUnaryExpression() {
        if (currentToken && currentToken->raw_val == "-") {
            advance();
            auto operand = ParseUnaryExpression();
            return std::make_unique<UnOp>(UnaryOperator::Neg, std::move(operand));
        }
        return ParsePrimaryExpression();
    }

    std::unique_ptr<Exp> ParsePrimaryExpression() {
        if (currentToken && currentToken->kind == TokenKind::TK_INT) {
            int value = std::stoi(currentToken->raw_val);
            advance();
            return std::make_unique<Literal>(value);
        }
        if (currentToken && currentToken->kind == TokenKind::TK_IDENTIFIER) {
            std::string name = currentToken->raw_val;
            advance();
            if (currentToken && currentToken->raw_val == "(") {
                // Function call or object creation
                return ParseFunctionOrObjectCreation(std::move(name));
            }
            if (currentToken && currentToken->raw_val == ".") {
                // Member access
                return ParseMemberAccess(std::make_unique<Var>(std::move(name)));
            }
            return std::make_unique<Var>(std::move(name));
        }
        if (currentToken && currentToken->raw_val == "(") {
            advance();
            auto exp = ParseExpression();
            Expect(")");
            return exp;
        }
        throw std::runtime_error("Unexpected token in primary expression in line: " + std::to_string(lexer.getCurrentLine()));
    }

    std::unique_ptr<Exp> ParseFunctionOrObjectCreation(std::string name) {
        Expect("(");
        std::vector<std::unique_ptr<Exp>> arguments;
        if (currentToken && currentToken->raw_val != ")") {
            do {
                arguments.push_back(ParseExpression());
            } while (currentToken && currentToken->raw_val == "," && (advance(), true));
        }
        Expect(")");
        if (IsTypeName(name)) {
            // Object creation
            return std::make_unique<ObjectCreation>(std::move(name), std::move(arguments));
        } else {
            // Function call
            return std::make_unique<FunctionCall>(std::move(name), std::move(arguments));
        }
    }

    std::unique_ptr<Exp> ParseMemberAccess(std::unique_ptr<Exp> object) {
        Expect(".");
        std::string memberName = currentToken->raw_val;
        advance();
        return std::make_unique<MemberAccess>(std::move(object), std::move(memberName));
    }

    bool IsTypeName(const std::string& name) {
        // Implement logic to check if 'name' is a type name (e.g., class or built-in type)
        // For simplicity, let's assume any name starting with a capital letter is a type
        return !name.empty() && std::isupper(name[0]);
    }
};

#endif //EXPRESSIONPARSER_HPP
