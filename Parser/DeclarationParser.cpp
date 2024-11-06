//
// Created by Elijah Crain on 11/5/24.
//

#ifndef DECLARATIONPARSER_HPP
#define DECLARATIONPARSER_HPP

#include "ParsingContext.hpp"
#include "ParserBase.hpp"
#include "StatementParser.hpp"
#include "ExpressionParser.hpp"
#include "TypeParser.cpp"

class DeclarationParser : public ParserBase {
public:
    explicit DeclarationParser(ParsingContext& context)
        : ParserBase(context), statementParser(context), expressionParser(context), typeParser(context) {}

    std::unique_ptr<Declaration> parseDeclaration() {
        if (isFunctionDeclaration()) {
            return parseFunctionDeclaration();
        } else if (isStructDeclaration()) {
            return parseStructDeclaration();
        } else {
            return parseVariableDeclaration();
        }
    }

private:
    StatementParser statementParser;
    ExpressionParser expressionParser;
    TypeParser typeParser;

    bool isFunctionDeclaration() {
        // Lookahead to determine if we're parsing a function declaration
        size_t savedPosition = context.getPosition();
        auto savedToken = context.currentToken;

        // Check for storage class specifier
        StorageClass storageClass = StorageClass::None;
        if (context.currentToken.raw_val == "static") {
            storageClass = StorageClass::Static;
            context.advance();
        } else if (context.currentToken.raw_val == "extern") {
            storageClass = StorageClass::Extern;
            context.advance();
        }

        // Parse return type
        auto returnType = typeParser.parseType();
        if (!returnType) {
            context.setPosition(savedPosition);
            context.currentToken = savedToken;
            return false;
        }

        // Expect identifier
        if (context.currentToken.kind != TokenKind::TK_IDENTIFIER) {
            context.setPosition(savedPosition);
            context.currentToken = savedToken;
            return false;
        }

        // Expect '('
        context.advance();
        if (context.currentToken.raw_val != "(") {
            context.setPosition(savedPosition);
            context.currentToken = savedToken;
            return false;
        }

        // It is a function declaration
        context.setPosition(savedPosition);
        context.currentToken = savedToken;
        return true;
    }

    bool isStructDeclaration() {
        return context.currentToken.raw_val == "struct";
    }

    std::unique_ptr<Declaration> parseFunctionDeclaration() {
        // Parse storage class specifier
        StorageClass storageClass = StorageClass::None;
        if (context.currentToken.raw_val == "static") {
            storageClass = StorageClass::Static;
            context.advance();
        } else if (context.currentToken.raw_val == "extern") {
            storageClass = StorageClass::Extern;
            context.advance();
        }

        // Parse return type
        auto returnType = typeParser.parseType();
        if (!returnType) {
            throw std::runtime_error("Expected return type in function declaration at line: " + std::to_string(context.getCurrentLine()));
        }

        // Parse function name
        if (context.currentToken.kind != TokenKind::TK_IDENTIFIER) {
            throw std::runtime_error("Expected function name in function declaration at line: " + std::to_string(context.getCurrentLine()));
        }
        std::string functionName = context.currentToken.raw_val;
        context.advance();

        // Parse parameter list
        Expect("(");
        std::vector<std::unique_ptr<Parameter>> parameters;
        if (context.currentToken.raw_val != ")") {
            do {
                auto paramType = typeParser.parseType();
                if (!paramType) {
                    throw std::runtime_error("Expected parameter type in function declaration at line: " + std::to_string(context.getCurrentLine()));
                }
                if (context.currentToken.kind != TokenKind::TK_IDENTIFIER) {
                    throw std::runtime_error("Expected parameter name in function declaration at line: " + std::to_string(context.getCurrentLine()));
                }
                std::string paramName = context.currentToken.raw_val;
                context.advance();
                parameters.push_back(std::make_unique<Parameter>(std::move(paramName), std::move(paramType)));
            } while (context.currentToken.raw_val == "," && (context.advance(), true));
        }
        Expect(")");

        // Parse function type
        auto funType = std::make_unique<FunType>(std::vector<std::unique_ptr<Type>>(), std::move(returnType));
        for (const auto& param : parameters) {
            funType->parameterTypes.push_back(param->paramType.clone());
        }

        // Parse function body (optional)
        std::optional<std::unique_ptr<Block>> body = std::nullopt;
        if (context.currentToken.raw_val == "{") {
            body = parseFunctionBody();
        } else {
            Expect(";");
        }

        return std::make_unique<FunctionDeclaration>(
            std::move(functionName),
            std::move(parameters),
            std::move(body),
            std::move(funType),
            storageClass
        );
    }

    std::unique_ptr<Block> parseFunctionBody() {
        Expect("{");
        std::vector<std::unique_ptr<BlockItem>> items;
        while (context.currentToken.raw_val != "}") {
            if (isDeclaration()) {
                auto decl = parseDeclaration();
                items.push_back(std::make_unique<BlockItemDeclaration>(std::move(decl)));
            } else {
                auto stmt = statementParser.parseStatement();
                items.push_back(std::make_unique<BlockItemStatement>(std::move(stmt)));
            }
        }
        Expect("}");
        return std::make_unique<Block>(std::move(items));
    }

    bool isDeclaration() {
        // Check if the next tokens form a type
        size_t savedPosition = context.getPosition();
        auto savedToken = context.currentToken;

        auto type = typeParser.parseType();
        bool isDecl = type != nullptr;

        context.setPosition(savedPosition);
        context.currentToken = savedToken;

        return isDecl;
    }

    std::unique_ptr<Declaration> parseVariableDeclaration() {
        // Parse storage class specifier
        StorageClass storageClass = StorageClass::None;
        if (context.currentToken.raw_val == "static") {
            storageClass = StorageClass::Static;
            context.advance();
        } else if (context.currentToken.raw_val == "extern") {
            storageClass = StorageClass::Extern;
            context.advance();
        }

        // Parse type
        auto varType = typeParser.parseType();
        if (!varType) {
            throw std::runtime_error("Expected type in variable declaration at line: " + std::to_string(context.getCurrentLine()));
        }

        // Parse variable name
        if (context.currentToken.kind != TokenKind::TK_IDENTIFIER) {
            throw std::runtime_error("Expected variable name in variable declaration at line: " + std::to_string(context.getCurrentLine()));
        }
        std::string varName = context.currentToken.raw_val;
        context.advance();

        // Parse initializer (optional)
        std::optional<std::unique_ptr<Initializer>> initializer = std::nullopt;
        if (context.currentToken.raw_val == "=") {
            context.advance();
            initializer = parseInitializer();
        }

        Expect(";");

        return std::make_unique<VariableDeclaration>(
            std::move(varName),
            std::move(varType),
            std::move(initializer),
            storageClass
        );
    }

    std::unique_ptr<Initializer> parseInitializer() {
        if (context.currentToken.raw_val == "{") {
            // Compound initializer
            context.advance();
            std::vector<std::unique_ptr<Initializer>> initializers;
            while (context.currentToken.raw_val != "}") {
                initializers.push_back(parseInitializer());
                if (context.currentToken.raw_val == ",") {
                    context.advance();
                }
            }
            Expect("}");
            return std::make_unique<CompoundInit>(std::move(initializers));
        } else {
            // Single initializer
            auto exp = expressionParser.parseExpression();
            return std::make_unique<SingleInit>(std::move(exp));
        }
    }

    std::unique_ptr<Declaration> parseStructDeclaration() {
        Expect("struct");
        if (context.currentToken.kind != TokenKind::TK_IDENTIFIER) {
            throw std::runtime_error("Expected struct tag in struct declaration at line: " + std::to_string(context.getCurrentLine()));
        }
        std::string tag = context.currentToken.raw_val;
        context.advance();

        Expect("{");
        std::vector<std::unique_ptr<MemberDeclaration>> members;
        while (context.currentToken.raw_val != "}") {
            auto memberType = typeParser.parseType();
            if (!memberType) {
                throw std::runtime_error("Expected member type in struct declaration at line: " + std::to_string(context.getCurrentLine()));
            }
            if (context.currentToken.kind != TokenKind::TK_IDENTIFIER) {
                throw std::runtime_error("Expected member name in struct declaration at line: " + std::to_string(context.getCurrentLine()));
            }
            std::string memberName = context.currentToken.raw_val;
            context.advance();
            Expect(";");
            members.push_back(std::make_unique<MemberDeclaration>(std::move(memberName), std::move(memberType)));
        }
        Expect("}");

        Expect(";");

        return std::make_unique<StructDeclaration>(std::move(tag), std::move(members));
    }
};

#endif // DECLARATIONPARSER_HPP
