#ifndef STATEMENTPARSER_HPP
#define STATEMENTPARSER_HPP

#include "ParsingContext.hpp"
#include "ParserBase.hpp"
#include "ExpressionParser.hpp"
#include "DeclarationParser.cpp"

class StatementParser : public ParserBase {
public:
    explicit StatementParser(ParsingContext& context)
        : ParserBase(context), expressionParser(context), declarationParser(context) {}

    std::unique_ptr<Statement> parseStatement() {
        if (context.currentToken.raw_val == "return") {
            return parseReturnStatement();
        } else if (context.currentToken.raw_val == "if") {
            return parseIfStatement();
        } else if (context.currentToken.raw_val == "while") {
            return parseWhileStatement();
        } else if (context.currentToken.raw_val == "do") {
            return parseDoWhileStatement();
        } else if (context.currentToken.raw_val == "for") {
            return parseForStatement();
        } else if (context.currentToken.raw_val == "break") {
            context.advance();
            Expect(";");
            return std::make_unique<BreakStatement>();
        } else if (context.currentToken.raw_val == "continue") {
            context.advance();
            Expect(";");
            return std::make_unique<ContinueStatement>();
        } else if (context.currentToken.raw_val == "{") {
            return parseCompoundStatement();
        } else if (context.currentToken.raw_val == ";") {
            context.advance();
            return std::make_unique<NullStatement>();
        } else if (isDeclaration()) {
            auto decl = declarationParser.parseVariableDeclaration();
            return std::make_unique<ExpressionStatement>(std::move(decl));
        } else {
            auto expr = expressionParser.parseExpression();
            Expect(";");
            return std::make_unique<ExpressionStatement>(std::move(expr));
        }
    }

private:
    ExpressionParser expressionParser;
    DeclarationParser declarationParser;

    bool isDeclaration() {
        // Lookahead to determine if this is a declaration
        size_t savedPosition = context.getPosition();
        auto savedToken = context.currentToken;

        auto type = typeParser.parseType();
        bool isDecl = type != nullptr;

        context.setPosition(savedPosition);
        context.currentToken = savedToken;

        return isDecl;
    }

    std::unique_ptr<Statement> parseReturnStatement() {
        context.advance();
        std::optional<std::unique_ptr<Exp>> expression = std::nullopt;
        if (context.currentToken.raw_val != ";") {
            expression = expressionParser.parseExpression();
        }
        Expect(";");
        return std::make_unique<ReturnStatement>(std::move(expression));
    }

    std::unique_ptr<Statement> parseIfStatement() {
        context.advance();
        Expect("(");
        auto condition = expressionParser.parseExpression();
        Expect(")");
        auto thenStmt = parseStatement();
        std::optional<std::unique_ptr<Statement>> elseStmt = std::nullopt;
        if (context.currentToken.raw_val == "else") {
            context.advance();
            elseStmt = parseStatement();
        }
        return std::make_unique<IfStatement>(std::move(condition), std::move(thenStmt), std::move(elseStmt));
    }

    std::unique_ptr<Statement> parseWhileStatement() {
        context.advance();
        Expect("(");
        auto condition = expressionParser.parseExpression();
        Expect(")");
        auto body = parseStatement();
        return std::make_unique<WhileStatement>(std::move(condition), std::move(body));
    }

    std::unique_ptr<Statement> parseDoWhileStatement() {
        context.advance();
        auto body = parseStatement();
        Expect("while");
        Expect("(");
        auto condition = expressionParser.parseExpression();
        Expect(")");
        Expect(";");
        return std::make_unique<DoWhileStatement>(std::move(body), std::move(condition));
    }

    std::unique_ptr<Statement> parseForStatement() {
        context.advance();
        Expect("(");
        auto init = parseForInit();
        Expect(";");
        std::optional<std::unique_ptr<Exp>> condition = std::nullopt;
        if (context.currentToken.raw_val != ";") {
            condition = expressionParser.parseExpression();
        }
        Expect(";");
        std::optional<std::unique_ptr<Exp>> post = std::nullopt;
        if (context.currentToken.raw_val != ")") {
            post = expressionParser.parseExpression();
        }
        Expect(")");
        auto body = parseStatement();
        return std::make_unique<ForStatement>(std::move(init), std::move(condition), std::move(post), std::move(body));
    }

    std::unique_ptr<Statement> parseCompoundStatement() {
        Expect("{");
        std::vector<std::unique_ptr<BlockItem>> items;
        while (context.currentToken.raw_val != "}") {
            if (isDeclaration()) {
                auto decl = declarationParser.parseDeclaration();
                items.push_back(std::make_unique<BlockItemDeclaration>(std::move(decl)));
            } else {
                auto stmt = parseStatement();
                items.push_back(std::make_unique<BlockItemStatement>(std::move(stmt)));
            }
        }
        Expect("}");
        return std::make_unique<CompoundStatement>(std::make_unique<Block>(std::move(items)));
    }

    std::unique_ptr<ForInit> parseForInit() {
        if (isDeclaration()) {
            auto decl = declarationParser.parseVariableDeclaration();
            return std::make_unique<InitDecl>(std::move(decl));
        } else {
            std::optional<std::unique_ptr<Exp>> expr = std::nullopt;
            if (context.currentToken.raw_val != ";") {
                expr = expressionParser.parseExpression();
            }
            return std::make_unique<InitExp>(std::move(expr));
        }
    }
};

#endif // STATEMENTPARSER_HPP
