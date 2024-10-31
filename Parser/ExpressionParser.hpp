//
// Created by Elijah on 10/29/2024.
//

// ExpressionParser.hpp

#ifndef EXPRESSIONPARSER_HPP
#define EXPRESSIONPARSER_HPP

#include "ParserBase.hpp"
#include "ParsingContext.hpp"

class ExpressionParser : public ParserBase {
public:
    std::unique_ptr<Exp> ParseExpression() {
        return ParseAdditiveExpression();
    }
    explicit ExpressionParser(ParsingContext& context) : ParserBase(context) {}

private:
    std::unique_ptr<Exp> ParseAdditiveExpression() {
        auto left = ParseMultiplicativeExpression();
        while (context.currentToken && (context.currentToken->raw_val == "+" || context.currentToken->raw_val == "-")) {
            BinaryOperator op = (context.currentToken->raw_val == "+") ? BinaryOperator::Add : BinaryOperator::Sub;
            context.advance();
            auto right = ParseMultiplicativeExpression();
            left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
        }
        return left;
    }

    std::unique_ptr<Exp> ParseMultiplicativeExpression() {
        auto left = ParseUnaryExpression();
        while (context.currentToken && (context.currentToken->raw_val == "*" || context.currentToken->raw_val == "/")) {
            BinaryOperator op = (context.currentToken->raw_val == "*") ? BinaryOperator::Mul : BinaryOperator::Div;
            context.advance();
            auto right = ParseUnaryExpression();
            left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
        }
        return left;
    }

    std::unique_ptr<Exp> ParseUnaryExpression() {
        if (context.currentToken && context.currentToken->raw_val == "-") {
            context.advance();
            auto operand = ParseUnaryExpression();
            return std::make_unique<UnOp>(UnaryOperator::Neg, std::move(operand));
        }
        return ParsePrimaryExpression();
    }

    std::unique_ptr<Exp> ParsePrimaryExpression() {
        if (context.currentToken && context.currentToken->kind == TokenKind::TK_INT) {
            int value = std::stoi(context.currentToken->raw_val);
            context.advance();
            return std::make_unique<Literal>(value);
        }
        if (context.currentToken && context.currentToken->kind == TokenKind::TK_IDENTIFIER) {
            std::string ns = "";
            std::string name = context.currentToken->raw_val;
            context.advance();
            // Handle namespace qualification
            while (context.currentToken && context.currentToken->raw_val == "::") {
                context.advance();
                if (ns.empty()) {
                    ns = name;
                } else {
                    ns += "::" + name;
                }
                if (context.currentToken && context.currentToken->kind == TokenKind::TK_IDENTIFIER) {
                    name = context.currentToken->raw_val;
                    context.advance();
                } else {
                    throw std::runtime_error("Expected identifier after '::' in line: " + std::to_string(context.getCurrentLine()));
                }
            }
            if (ns.empty()) {
                ns = context.getCurrentNamespace();
            }
            if (context.currentToken && context.currentToken->raw_val == "(") {
                // Function call or object creation
                return ParseFunctionOrObjectCreation(std::move(name), ns);
            }
            if (context.currentToken && context.currentToken->raw_val == ".") {
                // Member access
                return ParseMemberAccess(std::make_unique<Var>(std::move(name), ns));
            }
            return std::make_unique<Var>(std::move(name), ns);
        }
        if (context.currentToken && context.currentToken->raw_val == "(") {
            context.advance();
            auto exp = ParseExpression();
            Expect(")");
            return exp;
        }
        throw std::runtime_error("Unexpected token in primary expression in line: " + std::to_string(context.getCurrentLine()));
    }

    std::unique_ptr<Exp> ParseFunctionOrObjectCreation(std::string name, std::string ns = "") {
        Expect("(");
        std::vector<std::unique_ptr<Exp>> arguments;
        if (context.currentToken && context.currentToken->raw_val != ")") {
            do {
                arguments.push_back(ParseExpression());
            } while (context.currentToken && context.currentToken->raw_val == "," && (context.advance(), true));
        }
        Expect(")");
        if (IsTypeName(name)) {
            // Object creation
            return std::make_unique<ObjectCreation>(std::move(name), std::move(arguments));
        } else {
            // Function call
            return std::make_unique<FunctionCall>(std::move(name), std::move(ns), std::move(arguments));
        }
    }

    std::unique_ptr<Exp> ParseMemberAccess(std::unique_ptr<Exp> object) {
        Expect(".");
        std::string memberName = context.currentToken->raw_val;
        context.advance();
        return std::make_unique<MemberAccess>(std::move(object), std::move(memberName));
    }

    static bool IsTypeName(const std::string& name) {
        // Implement logic to check if 'name' is a type name (e.g., class or built-in type)
        // For simplicity, let's assume any name starting with a capital letter is a type
        return !name.empty() && std::isupper(name[0]);
    }
};

#endif //EXPRESSIONPARSER_HPP
