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

// Base Declaration Node
struct Declaration : ASTNode {};

// Program Node
struct Program : ASTNode {
    std::vector<std::unique_ptr<Declaration>> declarations;
    explicit Program(std::vector<std::unique_ptr<Declaration>> decls) : declarations(std::move(decls)) {}
};

// Type Hierarchy
struct Type {
    virtual ~Type() = default;
};

enum class PrimitiveType {
    Char,
    SChar,
    UChar,
    Int,
    UInt,
    Long,
    ULong,
    Double,
    Void,
};

struct PrimitiveTypeNode : Type {
    PrimitiveType type;
    explicit PrimitiveTypeNode(PrimitiveType t) : type(t) {}
};

struct FunType : Type {
    std::vector<std::unique_ptr<Type>> parameterTypes;
    std::unique_ptr<Type> returnType;
    FunType(std::vector<std::unique_ptr<Type>> params, std::unique_ptr<Type> ret)
        : parameterTypes(std::move(params)), returnType(std::move(ret)) {}
};

struct PointerType : Type {
    std::unique_ptr<Type> referencedType;
    explicit PointerType(std::unique_ptr<Type> refType) : referencedType(std::move(refType)) {}
};

struct ArrayType : Type {
    std::unique_ptr<Type> elementType;
    int size;
    ArrayType(std::unique_ptr<Type> elemType, int s) : elementType(std::move(elemType)), size(s) {}
};

struct StructType : Type {
    std::string tag;
    explicit StructType(std::string t) : tag(std::move(t)) {}
};

// Storage Class
enum class StorageClass {
    None,
    Static,
    Extern
};

// Initializer Hierarchy
struct Initializer {
    virtual ~Initializer() = default;
};

struct SingleInit : Initializer {
    std::unique_ptr<Exp> expression;
    explicit SingleInit(std::unique_ptr<Exp> e) : expression(std::move(e)) {}
};

struct CompoundInit : Initializer {
    std::vector<std::unique_ptr<Initializer>> initializers;
    explicit CompoundInit(std::vector<std::unique_ptr<Initializer>> inits) : initializers(std::move(inits)) {}
};

// Variable Declaration
struct VariableDeclaration : Declaration {
    std::string name;
    std::unique_ptr<Type> varType;
    std::optional<std::unique_ptr<Initializer>> initializer;
    StorageClass storageClass;
    VariableDeclaration(std::string n, std::unique_ptr<Type> t, std::optional<std::unique_ptr<Initializer>> init = std::nullopt, StorageClass sc = StorageClass::None)
        : name(std::move(n)), varType(std::move(t)), initializer(std::move(init)), storageClass(sc) {}
};

// Parameter
struct Parameter {
    std::string name;
    std::unique_ptr<Type> paramType;
    Parameter(std::string n, std::unique_ptr<Type> t) : name(std::move(n)), paramType(std::move(t)) {}
};

// Block Item
struct BlockItem {
    virtual ~BlockItem() = default;
};

struct BlockItemStatement : BlockItem {
    std::unique_ptr<Statement> statement;
    explicit BlockItemStatement(std::unique_ptr<Statement> s) : statement(std::move(s)) {}
};

struct BlockItemDeclaration : BlockItem {
    std::unique_ptr<Declaration> declaration;
    explicit BlockItemDeclaration(std::unique_ptr<Declaration> d) : declaration(std::move(d)) {}
};

// Block
struct Block : ASTNode {
    std::vector<std::unique_ptr<BlockItem>> items;
    explicit Block(std::vector<std::unique_ptr<BlockItem>> i) : items(std::move(i)) {}
};

// Function Declaration
struct FunctionDeclaration : Declaration {
    std::string name;
    std::vector<std::unique_ptr<Parameter>> parameters;
    std::optional<std::unique_ptr<Block>> body;
    std::unique_ptr<Type> funType;
    StorageClass storageClass;
    FunctionDeclaration(std::string n, std::vector<std::unique_ptr<Parameter>> params, std::optional<std::unique_ptr<Block>> b, std::unique_ptr<Type> ft, StorageClass sc = StorageClass::None)
        : name(std::move(n)), parameters(std::move(params)), body(std::move(b)), funType(std::move(ft)), storageClass(sc) {}
};

// Member Declaration
struct MemberDeclaration {
    std::string memberName;
    std::unique_ptr<Type> memberType;
    MemberDeclaration(std::string n, std::unique_ptr<Type> t) : memberName(std::move(n)), memberType(std::move(t)) {}
};

// Struct Declaration
struct StructDeclaration : Declaration {
    std::string tag;
    std::vector<std::unique_ptr<MemberDeclaration>> members;
    StructDeclaration(std::string t, std::vector<std::unique_ptr<MemberDeclaration>> m)
        : tag(std::move(t)), members(std::move(m)) {}
};

// For Initialization
struct ForInit {
    virtual ~ForInit() = default;
};

struct InitDecl : ForInit {
    std::unique_ptr<VariableDeclaration> declaration;
    explicit InitDecl(std::unique_ptr<VariableDeclaration> decl) : declaration(std::move(decl)) {}
};

struct InitExp : ForInit {
    std::optional<std::unique_ptr<Exp>> expression;
    explicit InitExp(std::optional<std::unique_ptr<Exp>> e = std::nullopt) : expression(std::move(e)) {}
};

// Statements
struct ReturnStatement : Statement {
    std::optional<std::unique_ptr<Exp>> expression;
    explicit ReturnStatement(std::optional<std::unique_ptr<Exp>> e = std::nullopt) : expression(std::move(e)) {}
};

struct ExpressionStatement : Statement {
    std::unique_ptr<Exp> expression;
    explicit ExpressionStatement(std::unique_ptr<Exp> e) : expression(std::move(e)) {}
};

struct IfStatement : Statement {
    std::unique_ptr<Exp> condition;
    std::unique_ptr<Statement> thenBranch;
    std::optional<std::unique_ptr<Statement>> elseBranch;
    IfStatement(std::unique_ptr<Exp> cond, std::unique_ptr<Statement> thenStmt, std::optional<std::unique_ptr<Statement>> elseStmt = std::nullopt)
        : condition(std::move(cond)), thenBranch(std::move(thenStmt)), elseBranch(std::move(elseStmt)) {}
};

struct CompoundStatement : Statement {
    std::unique_ptr<Block> block;
    explicit CompoundStatement(std::unique_ptr<Block> b) : block(std::move(b)) {}
};

struct BreakStatement : Statement {};

struct ContinueStatement : Statement {};

struct WhileStatement : Statement {
    std::unique_ptr<Exp> condition;
    std::unique_ptr<Statement> body;
    WhileStatement(std::unique_ptr<Exp> cond, std::unique_ptr<Statement> b)
        : condition(std::move(cond)), body(std::move(b)) {}
};

struct DoWhileStatement : Statement {
    std::unique_ptr<Statement> body;
    std::unique_ptr<Exp> condition;
    DoWhileStatement(std::unique_ptr<Statement> b, std::unique_ptr<Exp> cond)
        : body(std::move(b)), condition(std::move(cond)) {}
};

struct ForStatement : Statement {
    std::unique_ptr<ForInit> init;
    std::optional<std::unique_ptr<Exp>> condition;
    std::optional<std::unique_ptr<Exp>> post;
    std::unique_ptr<Statement> body;
    ForStatement(std::unique_ptr<ForInit> i, std::optional<std::unique_ptr<Exp>> cond, std::optional<std::unique_ptr<Exp>> p, std::unique_ptr<Statement> b)
        : init(std::move(i)), condition(std::move(cond)), post(std::move(p)), body(std::move(b)) {}
};

struct NullStatement : Statement {};

// Expressions

// Constants
enum class ConstantType {
    Int,
    Long,
    UInt,
    ULong,
    Double,
    Char,
    UChar
};

struct Constant : Exp {
    ConstantType type;
    union {
        double numericValue;
        char charValue;
    };
    explicit Constant(int v) : type(ConstantType::Int), numericValue(v) {}
    explicit Constant(long v) : type(ConstantType::Long), numericValue(v) {}
    explicit Constant(unsigned int v) : type(ConstantType::UInt), numericValue(v) {}
    explicit Constant(unsigned long v) : type(ConstantType::ULong), numericValue(v) {}
    explicit Constant(double v) : type(ConstantType::Double), numericValue(v) {}
    explicit Constant(char v) : type(ConstantType::Char), charValue(v) {}
    explicit Constant(unsigned char v) : type(ConstantType::UChar), charValue(v) {}
};

struct StringLiteral : Exp {
    std::string value;
    explicit StringLiteral(std::string v) : value(std::move(v)) {}
};

// Variable Expression
struct Var : Exp {
    std::string name;
    explicit Var(std::string n) : name(std::move(n)) {}
};

// Cast Expression
struct Cast : Exp {
    std::unique_ptr<Type> targetType;
    std::unique_ptr<Exp> expression;
    Cast(std::unique_ptr<Type> tt, std::unique_ptr<Exp> e) : targetType(std::move(tt)), expression(std::move(e)) {}
};

// Unary Operator Enum
enum class UnaryOperator {
    Negate,     // unary minus
    Complement, // bitwise NOT (~)
    Not         // logical NOT (!)
};

// Unary Operation Expression
struct UnOp : Exp {
    UnaryOperator op;
    std::unique_ptr<Exp> operand;
    UnOp(UnaryOperator o, std::unique_ptr<Exp> e) : op(o), operand(std::move(e)) {}
};

// Binary Operator Enum
enum class BinaryOperator {
    Add,
    Subtract,
    Multiply,
    Divide,
    Remainder,
    And,
    Or,
    Equal,
    NotEqual,
    LessThan,
    LessOrEqual,
    GreaterThan,
    GreaterOrEqual
};

// Binary Operation Expression
struct BinOp : Exp {
    BinaryOperator op;
    std::unique_ptr<Exp> lhs;
    std::unique_ptr<Exp> rhs;
    BinOp(BinaryOperator o, std::unique_ptr<Exp> l, std::unique_ptr<Exp> r)
        : op(o), lhs(std::move(l)), rhs(std::move(r)) {}
};

// Assignment Expression
struct Assignment : Exp {
    std::unique_ptr<Exp> lhs;
    std::unique_ptr<Exp> rhs;
    Assignment(std::unique_ptr<Exp> l, std::unique_ptr<Exp> r) : lhs(std::move(l)), rhs(std::move(r)) {}
};

// Conditional Expression
struct Conditional : Exp {
    std::unique_ptr<Exp> condition;
    std::unique_ptr<Exp> trueExp;
    std::unique_ptr<Exp> falseExp;
    Conditional(std::unique_ptr<Exp> cond, std::unique_ptr<Exp> tExp, std::unique_ptr<Exp> fExp)
        : condition(std::move(cond)), trueExp(std::move(tExp)), falseExp(std::move(fExp)) {}
};

// Function Call
struct FunctionCall : Exp {
    std::string name;
    std::vector<std::unique_ptr<Exp>> arguments;
    FunctionCall(std::string n, std::vector<std::unique_ptr<Exp>> args)
        : name(std::move(n)), arguments(std::move(args)) {}
};

// Dereference Expression
struct Dereference : Exp {
    std::unique_ptr<Exp> expression;
    explicit Dereference(std::unique_ptr<Exp> e) : expression(std::move(e)) {}
};

// Address Of Expression
struct AddrOf : Exp {
    std::unique_ptr<Exp> expression;
    explicit AddrOf(std::unique_ptr<Exp> e) : expression(std::move(e)) {}
};

// Subscript Expression
struct Subscript : Exp {
    std::unique_ptr<Exp> array;
    std::unique_ptr<Exp> index;
    Subscript(std::unique_ptr<Exp> arr, std::unique_ptr<Exp> idx)
        : array(std::move(arr)), index(std::move(idx)) {}
};

// SizeOf Expression
struct SizeOfExp : Exp {
    std::unique_ptr<Exp> expression;
    explicit SizeOfExp(std::unique_ptr<Exp> e) : expression(std::move(e)) {}
};

// SizeOf Type Expression
struct SizeOfType : Exp {
    std::unique_ptr<Type> type;
    explicit SizeOfType(std::unique_ptr<Type> t) : type(std::move(t)) {}
};

// Member Access (Dot)
struct Dot : Exp {
    std::unique_ptr<Exp> structure;
    std::string member;
    Dot(std::unique_ptr<Exp> s, std::string m) : structure(std::move(s)), member(std::move(m)) {}
};

// Member Access (Arrow)
struct Arrow : Exp {
    std::unique_ptr<Exp> pointer;
    std::string member;
    Arrow(std::unique_ptr<Exp> p, std::string m) : pointer(std::move(p)), member(std::move(m)) {}
};

#endif // PARSERTYPES_HPP
