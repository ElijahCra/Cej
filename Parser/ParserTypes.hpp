//
// Created by Elijah on 10/24/2024.
//

// ParserTypes.hpp

#ifndef PARSERTYPES_HPP
#define PARSERTYPES_HPP

#include <string>
#include <vector>
#include <memory>
#include <optional>

// Base AST Node
struct ASTNode {
    virtual ~ASTNode() = default;
};

// Base Expression Node
struct Exp : ASTNode {};
// Base Statement Node
struct Statement : ASTNode {};

// Program Node
struct CompilationUnit : ASTNode {
    std::vector<std::unique_ptr<Statement>> statements;
    explicit CompilationUnit(std::vector<std::unique_ptr<Statement>> s) : statements(std::move(s)) {}
};

// Function Node
struct FunctionDef : Statement {
    std::string name;
    int allocationSize;
    std::string returnType;
    std::vector<std::unique_ptr<Statement>> statements;
    std::string ns;

    FunctionDef(std::string n, int allocationSize, std::string rt, std::vector<std::unique_ptr<Statement>> s, std::string ns)
        : name(std::move(n)), allocationSize(allocationSize), returnType(std::move(rt)), statements(std::move(s)), ns(std::move(ns)) {}
};

// Return Statement
struct Return : Statement {
    std::unique_ptr<Exp> expression;
    explicit Return(std::unique_ptr<Exp> e) : expression(std::move(e)) {}
};

// Variable Declaration
struct Declare : Statement {
    std::string name;
    std::string type;
    std::optional<std::unique_ptr<Exp>> initializer;
    std::string ns;

    Declare(std::string n, std::string type, std::optional<std::unique_ptr<Exp>> i, std::string ns)
        : name(std::move(n)), type(std::move(type)), initializer(std::move(i)), ns(std::move(ns)) {}
};

// Expression Statement
struct ExpStatement : Statement {
    std::unique_ptr<Exp> expression;
    explicit ExpStatement(std::unique_ptr<Exp> e) : expression(std::move(e)) {}
};

// Class Definition
struct ClassDef : Statement {
    std::string name;
    std::vector<std::unique_ptr<Statement>> members;
    std::string ns;

    ClassDef(std::string n, std::vector<std::unique_ptr<Statement>> m, std::string ns)
        : name(std::move(n)), members(std::move(m)), ns(std::move(ns)) {}
};

// Namespace Definition
struct NamespaceDef : Statement {
    std::string name;
    std::vector<std::unique_ptr<Statement>> statements;
    NamespaceDef(std::string n, std::vector<std::unique_ptr<Statement>> s)
        : name(std::move(n)), statements(std::move(s)) {}
};

// Variable Expression
struct Var : Exp {
    std::string name;
    std::string ns;

    Var(std::string n, std::string ns) : name(std::move(n)), ns(std::move(ns)) {}
};

struct Assign : Exp {
    std::string name;
    std::string ns;
    std::unique_ptr<Exp> value;

    Assign(std::string n, std::string ns, std::unique_ptr<Exp> v) : name(std::move(n)), ns(std::move(ns)), value(std::move(v)) {}
};

// Function Call
struct FunctionCall : Exp {
    std::string name;
    std::string ns;
    std::vector<std::unique_ptr<Exp>> arguments;

    FunctionCall(std::string n, std::string ns, std::vector<std::unique_ptr<Exp>> args)
        : name(std::move(n)), ns(std::move(ns)), arguments(std::move(args)) {}
};

// Object Creation
struct ObjectCreation : Exp {
    std::string className;
    std::vector<std::unique_ptr<Exp>> arguments;
    ObjectCreation(std::string cn, std::vector<std::unique_ptr<Exp>> args)
        : className(std::move(cn)), arguments(std::move(args)) {}
};

// Member Access
struct MemberAccess : Exp {
    std::unique_ptr<Exp> object;
    std::string memberName;
    MemberAccess(std::unique_ptr<Exp> obj, std::string mn)
        : object(std::move(obj)), memberName(std::move(mn)) {}
};

// Binary Operator Enum
enum class BinaryOperator { Add, Sub, Mul, Div };

// Binary Operation Expression
struct BinOp : Exp {
    BinaryOperator op;
    std::unique_ptr<Exp> lhs;
    std::unique_ptr<Exp> rhs;
    BinOp(BinaryOperator o, std::unique_ptr<Exp> l, std::unique_ptr<Exp> r)
        : op(o), lhs(std::move(l)), rhs(std::move(r)) {}
};

// Unary Operator Enum
enum class UnaryOperator { Neg };

// Unary Operation Expression
struct UnOp : Exp {
    UnaryOperator op;
    std::unique_ptr<Exp> operand;
    UnOp(UnaryOperator o, std::unique_ptr<Exp> e) : op(o), operand(std::move(e)) {}
};

// Literal Expression
struct Literal : Exp {
    int value;
    explicit Literal(int v) : value(v) {}
};

#endif // PARSERTYPES_HPP
