//
// Created by Elijah on 8/13/2024.
//

#ifndef PARSER_CPP
#define PARSER_CPP
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include "Lexer.cpp"


struct Exp;
struct Statement;
struct Function;

struct Program {
    std::vector<std::unique_ptr<Function>> functions;
    explicit Program(std::vector<std::unique_ptr<Function>> f) : functions(std::move(f)) {}
};

struct Function {
    std::string name;
    int allocationSize;
    std::vector<std::unique_ptr<Statement>> statements;
    Function(std::string n, int allocationSize, std::vector<std::unique_ptr<Statement>> s)
        : name(std::move(n)), allocationSize(allocationSize), statements(std::move(s)) {}
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
    std::string type;
    std::optional<std::unique_ptr<Exp>> initializer;
    Declare(std::string n, std::string type,std::optional<std::unique_ptr<Exp>> i = std::nullopt)
        : name(std::move(n)), type(std::move(type)), initializer(std::move(i)) {}
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

struct FunctionCall : Exp {
    std::string name;
    std::vector<std::unique_ptr<Exp>> arguments;
    FunctionCall(std::string n, std::vector<std::unique_ptr<Exp>> args)
        : name(std::move(n)), arguments(std::move(args)) {}
};

enum class BinaryOperator { Add, Sub, Mul, Div };

struct BinOp : Exp {
    BinaryOperator op;
    std::unique_ptr<Exp> lhs;
    std::unique_ptr<Exp> rhs;
    BinOp(BinaryOperator o, std::unique_ptr<Exp> l, std::unique_ptr<Exp> r)
        : op(o), lhs(std::move(l)), rhs(std::move(r)) {}
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
    static std::unique_ptr<Program>
    Parse(std::unique_ptr<Token>& token) {
        return ParseProgram(token);
    }

    private:
    static constexpr std::array<std::string_view,1> systemTypes = {"int"};
    static constexpr std::array<std::pair<std::string,int>,1> typeSizes {std::pair("int",16)};

    static std::unique_ptr<Program>
    ParseProgram(std::unique_ptr<Token>& token) {
        std::vector<std::unique_ptr<Function>> functions;
        while (token->kind != TokenKind::TK_EOF) {
            functions.push_back(ParseFunction(token));
        }
        return std::make_unique<Program>(std::move(functions));
    }

    static std::unique_ptr<Function>
    ParseFunction(std::unique_ptr<Token>& token) {
        std::string name = token->literal;
        token = std::move(token->next);

        Expect(token, "::");
        Expect(token, "(");
        Expect(token, ")");
        Expect(token, "int");
        Expect(token, "{");

        int allocationSize = 0;
        std::vector<std::unique_ptr<Statement>> statements;
        while (!Equal(token.get(), "}")) {
            auto statement = ParseStatement(token);
            if (auto declare = dynamic_cast<Declare*>(statement.get())) {
                if (declare->type == "int") {
                    allocationSize += std::ranges::find(typeSizes, declare->type, &std::pair<std::string,int>::first)->second;
                }
            }
            statements.push_back(std::move(statement));
        }
        Expect(token, "}");

        return std::make_unique<Function>(std::move(name), allocationSize, std::move(statements));
    }

    static std::unique_ptr<Statement>
    ParseStatement(std::unique_ptr<Token>& token) {
        if (Equal(token.get(), "return")) {
            token = std::move(token->next);
            auto exp = ParseExpression(token);
            Expect(token, ";");
            return std::make_unique<Return>(std::move(exp));
        }
        if (token->kind == TokenKind::TK_IDENTIFIER) {
            std::string name = token->literal;
            token = std::move(token->next);

            if (Equal(token.get(), ":")) {
                token = std::move(token->next);
                if (std::ranges::find(systemTypes,token->literal) == systemTypes.end()) {
                    throw std::runtime_error("Provided type not in system types");
                }
                std::string type = token->literal;
                token = std::move(token->next);

                if (Equal(token.get(), "=")) {
                    token = std::move(token->next);
                    auto initializer = ParseExpression(token);
                    Expect(token, ";");
                    return std::make_unique<Declare>(std::move(name), std::move(type), std::move(initializer));
                }
                Expect(token, ";");
                return std::make_unique<Declare>(std::move(name), std::move(type));
            }

            if (Equal(token.get(), "=")) {
                token = std::move(token->next);
                auto exp = ParseExpression(token);
                Expect(token, ";");
                return std::make_unique<ExpStatement>(
                    std::make_unique<Assign>(std::move(name), std::move(exp))
                );
            }
            throw std::runtime_error("Unexpected token after identifier");
        }
        throw std::runtime_error("Unexpected statement");
    }

    static std::unique_ptr<Exp>
    ParseExpression(std::unique_ptr<Token>& token) {
        return ParseAdditiveExpression(token);
    }

    static std::unique_ptr<Exp>
    ParseAdditiveExpression(std::unique_ptr<Token>& token) {
        auto left = ParseMultiplicativeExpression(token);

        while (Equal(token.get(), "+") || Equal(token.get(), "-")) {
            BinaryOperator op = Equal(token.get(), "+") ? BinaryOperator::Add : BinaryOperator::Sub;
            token = std::move(token->next);
            auto right = ParseMultiplicativeExpression(token);
            left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
        }

        return left;
    }

    static std::unique_ptr<Exp>
    ParseMultiplicativeExpression(std::unique_ptr<Token>& token) {
        auto left = ParseUnaryExpression(token);

        while (Equal(token.get(), "*") || Equal(token.get(), "/")) {
            BinaryOperator op = Equal(token.get(), "*") ? BinaryOperator::Mul : BinaryOperator::Div;
            token = std::move(token->next);
            auto right = ParseUnaryExpression(token);
            left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
        }

        return left;
    }

    static std::unique_ptr<Exp>
    ParseUnaryExpression(std::unique_ptr<Token>& token) {
        if (Equal(token.get(), "-")) {
            token = std::move(token->next);
            auto operand = ParseUnaryExpression(token);
            return std::make_unique<UnOp>(UnaryOperator::Neg, std::move(operand));
        }
        return ParsePrimaryExpression(token);
    }

    static std::unique_ptr<Exp>
    ParsePrimaryExpression(std::unique_ptr<Token>& token) {
        if (token->kind == TokenKind::TK_INT) {
            int value = std::stoi(token->literal);
            token = std::move(token->next);
            return std::make_unique<Constant>(value);
        } else if (token->kind == TokenKind::TK_IDENTIFIER) {
            std::string name = token->literal;
            token = std::move(token->next);
            if (Equal(token.get(), "(")) {
                return ParseFunctionCall(name, token);
            }
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

    static std::unique_ptr<Exp>
    ParseFunctionCall(const std::string& name, std::unique_ptr<Token>& token) {
        Expect(token, "(");
        std::vector<std::unique_ptr<Exp>> arguments;
        if (!Equal(token.get(), ")")) {
            do {
                arguments.push_back(ParseExpression(token));
            } while (Equal(token.get(), ",") && (token = std::move(token->next)));
        }
        Expect(token, ")");
        return std::make_unique<FunctionCall>(name, std::move(arguments));
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
};

#endif //PARSER_CPP