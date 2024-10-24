//
// Created by Elijah on 10/24/2024.
//

#ifndef PARSERTYPES_HPP
#define PARSERTYPES_HPP

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

struct Literal : Exp {
    int value;
    explicit Literal(int v) : value(v) {}
};

#endif //PARSERTYPES_HPP
