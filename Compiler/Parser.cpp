//
// Created by Elijah on 8/13/2024.
//
#include <cassert>
#include <utility>
#ifndef PARSER_CPP
#define PARSER_CPP

#include <fstream>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include "Lexer.cpp"



struct StringHash {
    using is_transparent = void; // Enables heterogeneous operations.

    std::size_t operator()(std::string_view sv) const {
        std::hash<std::string_view> hasher;
        return hasher(sv);
    }
};

enum class NodeKind {
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_NEG,
	ND_INT,
    ND_VAR,
    ND_ASSIGN,
    ND_RETURN,
    ND_BLOCK,
    ND_FUNCTION,
    ND_VARDECL
};

struct Node {
    NodeKind kind;
    std::unique_ptr<Node> lhs;
    std::unique_ptr<Node> rhs;
    std::optional<int> value;
    std::string name;  // For var and function names
    std::vector<std::unique_ptr<Node>> block;  // For block statements
    std::vector<std::string> params;  // For function parameters
    std::string type;  // For variable and function return types
};

class Parser {

    public:
    static std::unordered_map<std::string, int, StringHash, std::equal_to<>> variables;
    static std::unique_ptr<Node>
    MakeNode(const NodeKind kind) {
            return std::make_unique<Node>(kind, nullptr, nullptr, std::nullopt);
        }

    static std::unique_ptr<Node>
    MakeUnaryNode(const NodeKind kind, std::unique_ptr<Node> lhs) {
        auto node = MakeNode(kind);
        node->lhs = std::move(lhs);
        return node;
    }

    static std::unique_ptr<Node>
    MakeBinaryNode(const NodeKind kind, std::unique_ptr<Node> lhs, std::unique_ptr<Node> rhs) {
        auto node = MakeNode(kind);
        node->lhs = std::move(lhs);
        node->rhs = std::move(rhs);
        return node;
    }

    static std::unique_ptr<Node>
    MakeIntegerNode(int value) {
        auto node = MakeNode(NodeKind::ND_INT);
        node->value = value;
        return node;
    }


    static bool
    Equal(const Token* token, const std::string_view str) {
        return token->literal == str;
    }

    static std::unique_ptr<Token>
    Expect(std::unique_ptr<Token>& token, const std::string_view expected) {
        if (!Equal(token.get(), expected)) {
            throw std::runtime_error("Expected '" + std::string(expected) + "', but got '" + token->literal + "'");
        }
        return std::exchange(token, std::move(token->next));
    }

    static std::unique_ptr<Node>
    ParseAdditiveExpression(std::unique_ptr<Token>& token) { using enum NodeKind;
        auto node = ParseMultiplicativeExpression(token);

        while (true) {
            if (Equal(token.get(), "+")) {
                auto next = std::move(token->next);
                node = MakeBinaryNode(ND_ADD, std::move(node), ParseMultiplicativeExpression(next));
                token = std::move(next);
            } else if (Equal(token.get(), "-")) {
                auto next = std::move(token->next);
                node = MakeBinaryNode(ND_SUB, std::move(node), ParseMultiplicativeExpression(next));
                token = std::move(next);
            } else {
                return node;
            }
        }
    }

    static std::unique_ptr<Node>
    ParseMultiplicativeExpression(std::unique_ptr<Token>& token) {
        using enum NodeKind;
        auto node = ParseUnaryExpression(token);

        while (true) {
            if (Equal(token.get(), "*")) {
                auto next = std::move(token->next);
                node = MakeBinaryNode(ND_MUL, std::move(node), ParseUnaryExpression(next));
                token = std::move(next);
            } else if (Equal(token.get(), "/")) {
                auto next = std::move(token->next);
                node = MakeBinaryNode(ND_DIV, std::move(node), ParseUnaryExpression(next));
                token = std::move(next);
            } else {
                return node;
            }
        }
    }

    static std::unique_ptr<Node>
    ParseUnaryExpression(std::unique_ptr<Token>& token) {
        using enum NodeKind;
        if (Equal(token.get(), "+")) {
            token = std::move(token->next);
            return ParseUnaryExpression(token);
        } else if (Equal(token.get(), "-")) {
            token = std::move(token->next);
            return MakeUnaryNode(ND_NEG, ParseUnaryExpression(token));
        } else {
            return ParsePrimaryExpression(token);
        }
    }

    static std::unique_ptr<Node>
    ParsePrimaryExpression(std::unique_ptr<Token>& token) {
        using enum TokenKind;
        if (Equal(token.get(), "(")) {
            token = std::move(token->next);
            auto node = ParseExpression(token);
            Expect(token, ")");
            return node;
        }
        if (token->kind == TK_INT) {
            auto node = MakeIntegerNode(std::stoi(token->literal));
            token = std::move(token->next);
            return node;
        }
        if (token->kind == TK_IDENTIFIER) {
            auto node = MakeNode(NodeKind::ND_VAR);
            node->name = token->literal;
            token = std::move(token->next);
            return node;
        }
        throw std::runtime_error("Expected an expression");
    }
    static std::unique_ptr<Node>
    ParseProgram(std::unique_ptr<Token>& token) { using enum TokenKind;
        auto program = MakeNode(NodeKind::ND_BLOCK);

        while (token->kind != TK_EOF) {
            if (token->kind == TK_IDENTIFIER) {
                if (token->next && token->next->kind == TK_COLONCOLON) {
                    program->block.push_back(ParseFunction(token));
                } else {
                    program->block.push_back(ParseVarDecl(token));
                }
            } else {
                throw std::runtime_error("Unexpected token at program level");
            }
        }

        return program;
    }

    static std::unique_ptr<Node>
    ParseVarDecl(std::unique_ptr<Token>& token) {
        auto node = MakeNode(NodeKind::ND_VARDECL);
        node->name = token->literal;
        token = std::move(token->next);

        Expect(token, ":");
        node->type = Expect(token, "int")->literal;

        Expect(token, "=");
        node->rhs = ParseExpression(token);

        Expect(token, ";");
        variables[node->name] = 0;  // Initialize variable
        return node;
    }

    static std::unique_ptr<Node>
    ParseFunction(std::unique_ptr<Token>& token) {
        auto func = MakeNode(NodeKind::ND_FUNCTION);
        func->name = token->literal;
        token = std::move(token->next);

        Expect(token, "::");
        Expect(token, "(");
        if (!Equal(token.get(), ")")) {
            do {
                func->params.push_back(token->literal);
                token = std::move(token->next);
                Expect(token, ":");
                Expect(token, "int");  // For now, only supporting int parameters
            } while (Equal(token.get(), ",") && (token = std::move(token->next)));
        }
        Expect(token, ")");

        func->type = Expect(token, "int")->literal;  // For now, only supporting int return type

        Expect(token, "{");
        while (!Equal(token.get(), "}")) {
            func->block.push_back(ParseStatement(token));
        }
        Expect(token, "}");

        return func;
    }

    static std::unique_ptr<Node>
ParseStatement(std::unique_ptr<Token>& token) {
        if (Equal(token.get(), "return")) {
            return ParseReturn(token);
        } else if (token->kind == TokenKind::TK_IDENTIFIER && token->next->kind == TokenKind::TK_COLON) {
            return ParseVarDecl(token);
        } else {
            auto node = ParseExpression(token);
            Expect(token, ";");
            return node;
        }
    }

    static std::unique_ptr<Node>
    ParseReturn(std::unique_ptr<Token>& token) {
        Expect(token, "return");
        auto node = MakeUnaryNode(NodeKind::ND_RETURN, ParseExpression(token));
        Expect(token, ";");
        return node;
    }

    static std::unique_ptr<Node>
    ParseExpression(std::unique_ptr<Token>& token) {
        auto node = ParseAdditiveExpression(token);

        if (Equal(token.get(), "=")) {
            token = std::move(token->next);
            node = MakeBinaryNode(NodeKind::ND_ASSIGN, std::move(node), ParseExpression(token));
        }

        return node;
    }

    static std::unique_ptr<Node> TreeFromTokens(std::unique_ptr<Token> tokens) {
        auto tree = ParseProgram(tokens);
        assert(tokens->kind == TokenKind::TK_EOF);
        return tree;
    }


};



#endif //PARSER_CPP