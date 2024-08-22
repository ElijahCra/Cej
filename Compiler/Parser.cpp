//
// Created by Elijah on 8/13/2024.
//
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>
#include <stdexcept>
#include "Lexer.cpp"
#include "Nodes.cpp"

enum class NodeKind {
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_NEG,
	ND_INT
};

struct Node {
	NodeKind kind;
	std::optional<Node*> lhs;
	std::optional<Node*> rhs;
	std::optional<int> value;
};

Node* MakeNode(const NodeKind kind) {
	return new Node{kind, std::nullopt, std::nullopt, std::nullopt};
}

Node *
MakeUnaryNode(NodeKind kind, Node *lhs)
{
    Node *node = MakeNode(kind);
    node->lhs = lhs;
    return node;
}

Node *
MakeBinaryNode(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = MakeNode(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *
MakeIntegerNode(int value)
{
    Node *node = MakeNode(NodeKind::ND_INT);
    node->value = value;
    return node;
}

Node *ParseAdditiveExpression(Token *token, Token **advance);
Node *ParseMultiplicativeExpression(Token *token, Token **advance);
Node *ParseUnaryExpression(Token *token, Token **advance);
Node *ParsePrimaryExpression(Token *token, Token **advance);

/*
 *  Additive = Multiplicative ( "+" Multiplicative | "+" Multiplicative ) *
 */
Node *
ParseAdditiveExpression(Token *token, Token **advance)
{
    Node *node = ParseMultiplicativeExpression(token, &token);

    while (true) {
	if (Equal(token, "+")) {
	    node = MakeBinaryNode(ND_ADD, node, ParseMultiplicativeExpression(token->next, &token));
	} else if (Equal(token, "-")) {
	    node = MakeBinaryNode(ND_SUB, node, ParseMultiplicativeExpression(token->next, &token));
	} else {
	    *advance = token;
	    return node;
	}
    }
}

/*
 *  Multiplicative = Unary ( "*" Unary | "/" Unary ) *
 */
Node *
ParseMultiplicativeExpression(Token *token, Token **advance)
{
    Node *node = ParseUnaryExpression(token, &token);

    while (true) {
	if (Equal(token, "*")) {
	    node = MakeBinaryNode(ND_MUL, node, ParseUnaryExpression(token->next, &token));
	} else if (Equal(token, "/")) {
	    node = MakeBinaryNode(ND_DIV, node, ParseUnaryExpression(token->next, &token));
	} else {
	    *advance = token;
	    return node;
	}
    }
}

/*
 *  Unary = ( "+" | "-" ) ( Unary | Primary )
 */
Node *
ParseUnaryExpression(Token *token, Token **advance)
{
    if (Equal(token, "+")) {
	return ParseUnaryExpression(token->next, advance);
    } else if (Equal(token, "-")) {
	return MakeUnaryNode(ND_NEG, ParseUnaryExpression(token->next, advance));
    } else {
	return ParsePrimaryExpression(token, advance);
    }
}

/*
 *  Primary = "(" Additive ")" | Integer
 */
Node *
ParsePrimaryExpression(Token *token, Token **advance)
{
    if (Equal(token, "(")) {
	Node *node = ParseAdditiveExpression(token->next, &token);
	*advance = Expect(token, ")");
	return node;
    }

    if (token->kind == TK_INT) {
	Node *node = MakeIntegerNode(token->value);
	*advance = token->next;
	return node;
    }

    Error("expected an expression");

    return NULL;
}

Node *
TreeFromTokens(Token **tokens)
{
    return ParseAdditiveExpression(*tokens, tokens);
}