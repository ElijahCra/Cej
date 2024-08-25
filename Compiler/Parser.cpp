#ifndef PARSER_CPP
#define PARSER_CPP
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "Lexer.cpp"

// Forward declarations
struct Exp;
struct Statement;

// AST node structures
struct Program {
    std::unique_ptr<struct Function> function;
    explicit Program(std::unique_ptr<struct Function> f) : function(std::move(f)) {}
};

struct Function {
    std::string name;
    std::vector<std::unique_ptr<Statement>> statements;
    Function(std::string n, std::vector<std::unique_ptr<Statement>> s) 
        : name(std::move(n)), statements(std::move(s)) {}
};

struct Statement {
    virtual ~Statement() = default;
};

struct Return : Statement {
    std::unique_ptr<Exp> expression;
    explicit Return(std::unique_ptr<Exp> e) : expression(std::move(e)) {}
};

struct Declare : Statement {
    std::string name;
    std::optional<std::unique_ptr<Exp>> initializer;
    Declare(std::string n, std::optional<std::unique_ptr<Exp>> i = std::nullopt) 
        : name(std::move(n)), initializer(std::move(i)) {}
};

struct ExpStatement : Statement {
    std::unique_ptr<Exp> expression;
    explicit ExpStatement(std::unique_ptr<Exp> e) : expression(std::move(e)) {}
};

struct Exp {
    virtual ~Exp() = default;
};

struct Assign : Exp {
    std::string name;
    std::unique_ptr<Exp> value;
    Assign(std::string n, std::unique_ptr<Exp> v) : name(std::move(n)), value(std::move(v)) {}
};

struct Var : Exp {
    std::string name;
    explicit Var(std::string n) : name(std::move(n)) {}
};

enum class BinaryOperator { Add, Sub, Mul, Div };

struct BinOp : Exp {
    BinaryOperator op;
    std::unique_ptr<Exp> left;
    std::unique_ptr<Exp> right;
    BinOp(BinaryOperator o, std::unique_ptr<Exp> l, std::unique_ptr<Exp> r)
        : op(o), left(std::move(l)), right(std::move(r)) {}
};

enum class UnaryOperator { Neg };

struct UnOp : Exp {
    UnaryOperator op;
    std::unique_ptr<Exp> operand;
    UnOp(UnaryOperator o, std::unique_ptr<Exp> e) : op(o), operand(std::move(e)) {}
};

struct Constant : Exp {
    int value;
    explicit Constant(int v) : value(v) {}
};

class Parser {
public:
    static std::unique_ptr<Program> Parse(std::unique_ptr<Token>& token) {
        return ParseProgram(token);
    }

private:
    static std::unique_ptr<Program> ParseProgram(std::unique_ptr<Token>& token) {
        auto function = ParseFunction(token);
        return std::make_unique<Program>(std::move(function));
    }

    static std::unique_ptr<Function> ParseFunction(std::unique_ptr<Token>& token) {
        std::string name = token->literal;
        token = std::move(token->next);

        Expect(token, "::");
        Expect(token, "(");
        Expect(token, ")");
        Expect(token, "int");
        Expect(token, "{");

        std::vector<std::unique_ptr<Statement>> statements;
        while (!Equal(token.get(), "}")) {
            statements.push_back(ParseStatement(token));
        }
        Expect(token, "}");

        return std::make_unique<Function>(std::move(name), std::move(statements));
    }

    static std::unique_ptr<Statement> ParseStatement(std::unique_ptr<Token>& token) {
        if (Equal(token.get(), "return")) {
            token = std::move(token->next);
            auto exp = ParseExpression(token);
            Expect(token, ";");
            return std::make_unique<Return>(std::move(exp));
        } else if (token->kind == TokenKind::TK_IDENTIFIER) {
            std::string name = token->literal;
            token = std::move(token->next);
            
            if (Equal(token.get(), "=")) {
                token = std::move(token->next);
                auto exp = ParseExpression(token);
                Expect(token, ";");
                return std::make_unique<ExpStatement>(
                    std::make_unique<Assign>(std::move(name), std::move(exp))
                );
            } else {
                throw std::runtime_error("Unexpected token after identifier");
            }
        } else {
            throw std::runtime_error("Unexpected statement");
        }
    }

    static std::unique_ptr<Exp> ParseExpression(std::unique_ptr<Token>& token) {
        return ParseAdditiveExpression(token);
    }

    static std::unique_ptr<Exp> ParseAdditiveExpression(std::unique_ptr<Token>& token) {
        auto left = ParseMultiplicativeExpression(token);

        while (Equal(token.get(), "+") || Equal(token.get(), "-")) {
            BinaryOperator op = Equal(token.get(), "+") ? BinaryOperator::Add : BinaryOperator::Sub;
            token = std::move(token->next);
            auto right = ParseMultiplicativeExpression(token);
            left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
        }

        return left;
    }

    static std::unique_ptr<Exp> ParseMultiplicativeExpression(std::unique_ptr<Token>& token) {
        auto left = ParseUnaryExpression(token);

        while (Equal(token.get(), "*") || Equal(token.get(), "/")) {
            BinaryOperator op = Equal(token.get(), "*") ? BinaryOperator::Mul : BinaryOperator::Div;
            token = std::move(token->next);
            auto right = ParseUnaryExpression(token);
            left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
        }

        return left;
    }

    static std::unique_ptr<Exp> ParseUnaryExpression(std::unique_ptr<Token>& token) {
        if (Equal(token.get(), "-")) {
            token = std::move(token->next);
            auto operand = ParseUnaryExpression(token);
            return std::make_unique<UnOp>(UnaryOperator::Neg, std::move(operand));
        } else {
            return ParsePrimaryExpression(token);
        }
    }

    static std::unique_ptr<Exp> ParsePrimaryExpression(std::unique_ptr<Token>& token) {
        if (token->kind == TokenKind::TK_INT) {
            int value = std::stoi(token->literal);
            token = std::move(token->next);
            return std::make_unique<Constant>(value);
        } else if (token->kind == TokenKind::TK_IDENTIFIER) {
            std::string name = token->literal;
            token = std::move(token->next);
            return std::make_unique<Var>(std::move(name));
        } else if (Equal(token.get(), "(")) {
            token = std::move(token->next);
            auto exp = ParseExpression(token);
            Expect(token, ")");
            return exp;
        } else {
            throw std::runtime_error("Unexpected token in primary expression");
        }
    }

    static bool Equal(const Token* token, const std::string_view str) {
        return token->literal == str;
    }

    static std::unique_ptr<Token> Expect(std::unique_ptr<Token>& token, const std::string_view expected) {
        if (!Equal(token.get(), expected)) {
            throw std::runtime_error("Expected '" + std::string(expected) + "', but got '" + token->literal + "'");
        }
        return std::exchange(token, std::move(token->next));
    }
};

#endif //PARSER_CPP