//
// Created by Elijah Crain on 10/29/24.
//

// StatementParser.hpp

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
            std::string ns = "";
            std::string name = context.currentToken->raw_val;
            context.advance();

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

            if (context.currentToken && context.currentToken->raw_val == ":") {
                context.advance();
                if (context.currentToken && context.currentToken->raw_val == "int") {
                    std::string type = context.currentToken->raw_val;
                    context.advance();

                    if (context.currentToken && context.currentToken->raw_val == "=") {
                        context.advance();
                        auto initializer = expressionParser.ParseExpression();
                        Expect(";");
                        return std::make_unique<Declare>(std::move(name), std::move(type), std::move(initializer), ns);
                    }
                    Expect(";");
                    return std::make_unique<Declare>(std::move(name), std::move(type), std::nullopt, ns);
                }
                throw std::runtime_error("Provided type not in system types in line: " + std::to_string(context.getCurrentLine()));
            }
            if (context.currentToken && context.currentToken->raw_val == "=") {
                context.advance();
                auto exp = expressionParser.ParseExpression();
                Expect(";");
                return std::make_unique<ExpStatement>(std::make_unique<Assign>(std::move(name), ns.empty() ? context.getCurrentNamespace() : ns, std::move(exp)));
            }
            throw std::runtime_error("Unexpected token after identifier in line: " + std::to_string(context.getCurrentLine()));
        }
        if (context.currentToken && context.currentToken->raw_val == "int") {
            return ParseVariableDeclaration();
        }
        throw std::runtime_error("Unexpected statement in line: " + std::to_string(context.getCurrentLine()) + " at position: " + std::to_string(context.getCurrentPosition()));
    }

private:
    std::unique_ptr<Statement> ParseVariableDeclaration() {
        std::string type = context.currentToken->raw_val;
        context.advance();

        std::string ns = "";
        std::string name = context.currentToken->raw_val;
        context.advance();

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

        std::optional<std::unique_ptr<Exp>> initializer = std::nullopt;
        if (context.currentToken && context.currentToken->raw_val == "=") {
            context.advance();
            initializer = expressionParser.ParseExpression();
        }
        Expect(";");
        return std::make_unique<Declare>(std::move(name), std::move(type), std::move(initializer), ns.empty() ? context.getCurrentNamespace() : ns);
    }

    std::unique_ptr<Statement> ParseClassDef() {
        Expect("class");
        std::string name = context.currentToken->raw_val;
        context.advance();

        std::string ns = context.getCurrentNamespace();

        Expect("{");
        context.enterNamespace(name); // Enter class namespace

        std::vector<std::unique_ptr<Statement>> members;
        while (context.currentToken && context.currentToken->raw_val != "}") {
            members.push_back(ParseStatement());
        }
        Expect("}");

        context.exitNamespace(); // Exit class namespace

        return std::make_unique<ClassDef>(std::move(name), std::move(members), ns);
    }

    std::unique_ptr<Statement> ParseNamespaceDef() {
        Expect("namespace");
        std::string name = context.currentToken->raw_val;
        context.advance();
        Expect("{");

        context.enterNamespace(name); // Enter namespace

        std::vector<std::unique_ptr<Statement>> statements;
        while (context.currentToken && context.currentToken->raw_val != "}") {
            statements.push_back(ParseStatement());
        }
        Expect("}");

        context.exitNamespace(); // Exit namespace

        return std::make_unique<NamespaceDef>(std::move(name), std::move(statements));
    }
};

#endif //STATEMENTPARSER_HPP
