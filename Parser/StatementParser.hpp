//
// Created by Elijah Crain on 10/29/24.
//

#ifndef STATEMENTPARSER_HPP
#define STATEMENTPARSER_HPP

#include "ParsingContext.hpp"
#include "ParserBase.hpp"
#include "ExpressionParser.hpp"

class StatementParser : public ParserBase {
    ExpressionParser expressionParser;
public:
    explicit StatementParser(ParsingContext& context) : ParserBase(context), expressionParser(context) {
    }

    std::unique_ptr<Statement> ParseStatement() {
        if (context.currentToken && context.currentToken->raw_val == "return") {
            context.advance();
            auto exp = expressionParser.ParseExpression();
            Expect(";");
            return std::make_unique<Return>(std::move(exp));
        }
        if (context.currentToken && context.currentToken->raw_val == "class") {
            return ParseClassDef();
        }
        if (context.currentToken && context.currentToken->raw_val == "namespace") {
            return ParseNamespaceDef();
        }
        if (context.currentToken && context.currentToken->kind == TokenKind::TK_IDENTIFIER) {
            return ParseExpressionStatement();
        }
        if (context.currentToken && (context.currentToken->raw_val == "int" || context.currentToken->raw_val == "string" || context.currentToken->raw_val == "bool")) {
            return ParseVariableDeclaration();
        }

        throw std::runtime_error("Unexpected statement in line: " + std::to_string(context.getCurrentLine()) + " at position: " + std::to_string(context.getCurrentPosition()));
    }

private:
    std::unique_ptr<Statement> ParseVariableDeclaration() {
        std::string type = context.currentToken->raw_val;
        context.advance();
        std::string name = context.currentToken->raw_val;
        context.advance();
        std::optional<std::unique_ptr<Exp>> initializer = std::nullopt;
        if (context.currentToken && context.currentToken->raw_val == "=") {
            context.advance();
            initializer = expressionParser.ParseExpression();
        }
        Expect(";");
        return std::make_unique<Declare>(std::move(name), std::move(type), std::move(initializer));
    }

    std::unique_ptr<Statement> ParseExpressionStatement() {
        auto exp = expressionParser.ParseExpression();
        Expect(";");
        return std::make_unique<ExpStatement>(std::move(exp));
    }

    std::unique_ptr<Statement> ParseClassDef() {
        Expect("class");
        std::string name = context.currentToken->raw_val;
        context.advance();
        Expect("{");
        std::vector<std::unique_ptr<Statement>> members;
        while (context.currentToken && context.currentToken->raw_val != "}") {
            members.push_back(ParseStatement());
        }
        Expect("}");
        return std::make_unique<ClassDef>(std::move(name), std::move(members));
    }

    std::unique_ptr<Statement> ParseNamespaceDef() {
        Expect("namespace");
        std::string name = context.currentToken->raw_val;
        context.advance();
        Expect("{");
        std::vector<std::unique_ptr<Statement>> statements;
        while (context.currentToken && context.currentToken->raw_val != "}") {
            statements.push_back(ParseStatement());
        }
        Expect("}");
        return std::make_unique<NamespaceDef>(std::move(name), std::move(statements));
    }
};

#endif //STATEMENTPARSER_HPP
