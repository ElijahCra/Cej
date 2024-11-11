//
// Created by Elijah on 11/10/2024.
//
#include "ExpressionParser.hpp"

std::unique_ptr<Exp>
ExpressionParser::ParseExpression() {
    return ParseAssignmentExpression();
}

std::unique_ptr<Exp>
ExpressionParser::ParseAssignmentExpression() {
    auto lhs = ParseLogicalOrExpression();
    if (context.currentToken.raw_val == "=") {
        context.advance();
        auto rhs = ParseAssignmentExpression();
        return std::make_unique<Assignment>(std::move(lhs), std::move(rhs));
    }
    return lhs;
}

std::unique_ptr<Exp>
ExpressionParser::ParseLogicalOrExpression() {
    auto left = ParseLogicalAndExpression();
    while (context.currentToken.raw_val == "||") {
        context.advance();
        auto right = ParseLogicalAndExpression();
        left = std::make_unique<BinOp>(BinaryOperator::Or, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Exp>
ExpressionParser::ParseLogicalAndExpression() {
    auto left = ParseEqualityExpression();
    while (context.currentToken.raw_val == "&&") {
        context.advance();
        auto right = ParseEqualityExpression();
        left = std::make_unique<BinOp>(BinaryOperator::And, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Exp>
ExpressionParser::ParseEqualityExpression() {
    auto left = ParseRelationalExpression();
    while (context.currentToken.raw_val == "==" || context.currentToken.raw_val == "!=") {
        BinaryOperator op = (context.currentToken.raw_val == "==") ? BinaryOperator::Equal : BinaryOperator::NotEqual;
        context.advance();
        auto right = ParseRelationalExpression();
        left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Exp>
ExpressionParser::ParseRelationalExpression() {
    auto left = ParseAdditiveExpression();
    while (context.currentToken.raw_val == "<" || context.currentToken.raw_val == "<=" ||
           context.currentToken.raw_val == ">" || context.currentToken.raw_val == ">=") {
        BinaryOperator op;
        if (context.currentToken.raw_val == "<")
            op = BinaryOperator::LessThan;
        else if (context.currentToken.raw_val == "<=")
            op = BinaryOperator::LessOrEqual;
        else if (context.currentToken.raw_val == ">")
            op = BinaryOperator::GreaterThan;
        else // context.currentToken.raw_val == ">="
            op = BinaryOperator::GreaterOrEqual;

        context.advance();
        auto right = ParseAdditiveExpression();
        left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Exp>
ExpressionParser::ParseAdditiveExpression() {
    auto left = ParseMultiplicativeExpression();
    while (context.currentToken.raw_val == "+" || context.currentToken.raw_val == "-") {
        BinaryOperator op = (context.currentToken.raw_val == "+") ? BinaryOperator::Add : BinaryOperator::Subtract;
        context.advance();
        auto right = ParseMultiplicativeExpression();
        left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Exp>
ExpressionParser::ParseMultiplicativeExpression() {
    auto left = ParseUnaryExpression();
    while (context.currentToken.raw_val == "*" || context.currentToken.raw_val == "/" || context.currentToken.raw_val == "%") {
        BinaryOperator op;
        if (context.currentToken.raw_val == "*")
            op = BinaryOperator::Multiply;
        else if (context.currentToken.raw_val == "/")
            op = BinaryOperator::Divide;
        else // context.currentToken.raw_val == "%"
            op = BinaryOperator::Remainder;

        context.advance();
        auto right = ParseUnaryExpression();
        left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Exp>
ExpressionParser::ParseUnaryExpression() {
    if (context.currentToken.raw_val == "-" || context.currentToken.raw_val == "~" || context.currentToken.raw_val == "!") {
        UnaryOperator op;
        if (context.currentToken.raw_val == "-")
            op = UnaryOperator::Negate;
        else if (context.currentToken.raw_val == "~")
            op = UnaryOperator::Complement;
        else // context.currentToken.raw_val == "!"
            op = UnaryOperator::Not;

        context.advance();
        auto operand = ParseUnaryExpression();
        return std::make_unique<UnOp>(op, std::move(operand));
    }
    return ParsePrimaryExpression();
}

std::unique_ptr<Exp>
ExpressionParser::ParsePrimaryExpression() {
    if (context.currentToken.kind == TokenKind::TK_INT) {
        int value = std::stoi(context.currentToken.raw_val);
        context.advance();
        return std::make_unique<Constant>(value);
    }
    if (context.currentToken.kind == TokenKind::TK_IDENTIFIER) {
        std::string name = context.currentToken.raw_val;
        context.advance();
        if (context.currentToken.raw_val == "(") {
            // Function call
            return ParseFunctionCall(std::move(name));
        }
        return std::make_unique<Var>(std::move(name));
    }
    if (context.currentToken.raw_val == "(") {
        context.advance();
        auto exp = ParseExpression();
        Expect(")");
        return exp;
    }
    throw std::runtime_error("Unexpected token in primary expression at line: " + std::to_string(context.getCurrentLine()));
}

std::unique_ptr<Exp>
ExpressionParser::ParseFunctionCall(std::string name) {
    Expect("(");
    std::vector<std::unique_ptr<Exp>> arguments;
    if (context.currentToken.raw_val != ")") {
        do {
            arguments.push_back(ParseExpression());
        } while (context.currentToken.raw_val == "," && (context.advance(), true));
    }
    Expect(")");
    return std::make_unique<FunctionCall>(std::move(name), std::move(arguments));
}

bool
ExpressionParser::IsTypeName(const std::string& name) const {
    return context.IsTypeName(name);
}
