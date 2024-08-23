//
// Created by Elijah on 8/13/2024.
//
module;
#ifndef PARSER_CPP
#define PARSER_CPP


#include <cassert>
#include <fstream>
#include <optional>
#include <stdexcept>
import Lexer;
export module Parser;
export enum class NodeKind {
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_NEG,
	ND_INT
};

export struct Node {
	NodeKind kind;
	std::unique_ptr<Node> lhs;
	std::unique_ptr<Node> rhs;
	std::optional<int> value;
};

export class Parser {
    public:

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
        return token->kind == TokenKind::TK_PNT && token->literal == str;
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
            auto node = ParseAdditiveExpression(token);
            Expect(token, ")");
            return node;
        }
        if (token->kind == TK_INT) {
            auto node = MakeIntegerNode(std::stoi(token->literal));
            token = std::move(token->next);
            return node;
        }
        throw std::runtime_error("Expected an expression");
    }
    static std::unique_ptr<Node> TreeFromTokens(std::unique_ptr<Token> tokens) {
        auto tree = ParseAdditiveExpression(tokens);
        assert(tokens->kind == TokenKind::TK_EOF);
        return tree;
    }
};



#endif //PARSER_CPP