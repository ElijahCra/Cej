//
// Created by Elijah on 8/13/2024.
//

#include <sys/stat.h>
#ifndef GENERATOR_CPP
#define GENERATOR_CPP

#include <sstream>
#include <unordered_map>
#include "../Parser/Parser.cpp"

class Generator {
    inline static std::stringstream assembly;
    inline static std::unordered_map<std::string, std::unordered_map<std::string, int>> functionVariables;
    inline static int stackSize;
    inline static int labelCounter;
    inline static std::string currentFunction;

    static void
    EmitLine(const std::string& line) {
        assembly << line << "\n";
    }

    static void
    AllocateVariable(const std::string& name) {
        stackSize += 16;
        functionVariables[currentFunction][name] = -stackSize;
    }

    static void
    GenerateExp(const std::unique_ptr<Exp>& exp) {
        if (auto literal = dynamic_cast<Literal*>(exp.get())) {
            EmitLine("\tmov x0, #" + std::to_string(literal->value));
        } else if (auto var = dynamic_cast<Var*>(exp.get())) {
            int offset = functionVariables[currentFunction][var->name];
            EmitLine("\tldr x0, [x29, #" + std::to_string(offset) + "]");
        } else if (auto binOp = dynamic_cast<BinOp*>(exp.get())) {
            bool lhsSimple = dynamic_cast<Var*>(binOp->lhs.get()) || dynamic_cast<Literal*>(binOp->lhs.get());
            bool rhsSimple = dynamic_cast<Var*>(binOp->rhs.get()) || dynamic_cast<Literal*>(binOp->rhs.get());

            if (lhsSimple && rhsSimple) {
                if (auto rhsVar = dynamic_cast<Var*>(binOp->rhs.get())) {
                    int offset = functionVariables[currentFunction][rhsVar->name];
                    EmitLine("\tldr x0, [x29, #" + std::to_string(offset) + "]");
                } else if (auto rhsLiteral = dynamic_cast<Literal*>(binOp->rhs.get())) {
                    EmitLine("\tmov x0, #" + std::to_string(rhsLiteral->value));
                }

                if (auto lhsVar = dynamic_cast<Var*>(binOp->lhs.get())) {
                    int offset = functionVariables[currentFunction][lhsVar->name];
                    EmitLine("\tldr x1, [x29, #" + std::to_string(offset) + "]");
                } else if (auto lhsLiteral = dynamic_cast<Literal*>(binOp->lhs.get())) {
                    EmitLine("\tmov x1, #" + std::to_string(lhsLiteral->value));
                }
            } else if (lhsSimple) {
                GenerateExp(binOp->rhs);
                if (auto lhsVar = dynamic_cast<Var*>(binOp->lhs.get())) {
                    int offset = functionVariables[currentFunction][lhsVar->name];
                    EmitLine("\tldr x1, [x29, #" + std::to_string(offset) + "]");
                } else if (auto lhsLiteral = dynamic_cast<Literal*>(binOp->lhs.get())) {
                    EmitLine("\tmov x1, #" + std::to_string(lhsLiteral->value));
                }
            } else if (rhsSimple) {
                GenerateExp(binOp->lhs);
                if (auto rhsVar = dynamic_cast<Var*>(binOp->rhs.get())) {
                    int offset = functionVariables[currentFunction][rhsVar->name];
                    EmitLine("\tldr x1, [x29, #" + std::to_string(offset) + "]");
                } else if (auto rhsLiteral = dynamic_cast<Literal*>(binOp->rhs.get())) {
                    EmitLine("\tmov x1, #" + std::to_string(rhsLiteral->value));
                }
            } else {
                GenerateExp(binOp->rhs);
                EmitLine("\tstr x0, [sp, #-16]!");
                GenerateExp(binOp->lhs);
                EmitLine("\tldr x1, [sp], #16");

            }
            switch (binOp->op) {
                case BinaryOperator::Add:
                    EmitLine("\tadd x0, x0, x1");
                break;
                case BinaryOperator::Sub:
                    EmitLine("\tsub x0, x0, x1");
                break;
                case BinaryOperator::Mul:
                    EmitLine("\tmul x0, x0, x1");
                break;
                case BinaryOperator::Div:
                    EmitLine("\tsdiv x0, x0, x1");
                break;
            }
        } else if (auto unOp = dynamic_cast<UnOp*>(exp.get())) {
            GenerateExp(unOp->operand);
            if (unOp->op == UnaryOperator::Neg) {
                EmitLine("\tneg x0, x0");
            }
        } else if (auto assign = dynamic_cast<Assign*>(exp.get())) {
            GenerateExp(assign->value);
            int offset = functionVariables[currentFunction][assign->name];
            EmitLine("\tstr x0, [x29, #" + std::to_string(offset) + "]");
        } else if (auto funcCall = dynamic_cast<FunctionCall*>(exp.get())) {
            if (funcCall->arguments.size() % 2 != 0) {
                EmitLine("\tsub sp, sp, #16");
            }
            for (const auto& arg : funcCall->arguments) {
                GenerateExp(arg);
                EmitLine("\tstr x0, [sp, #-16]!");
            }
            EmitLine("\tbl _" + funcCall->name);
        }
    }

    static void
    GenerateStatement(const std::unique_ptr<Statement>& stmt) {
        if (auto returnStmt = dynamic_cast<Return*>(stmt.get())) {
            GenerateExp(returnStmt->expression);
        } else if (auto declareStmt = dynamic_cast<Declare*>(stmt.get())) {
            AllocateVariable(declareStmt->name);
            if (declareStmt->initializer) {
                GenerateExp(*declareStmt->initializer);
                int offset = functionVariables[currentFunction][declareStmt->name];
                EmitLine("\tstr x0, [x29, #" + std::to_string(offset) + "]");
            }
        } else if (auto expStmt = dynamic_cast<ExpStatement*>(stmt.get())) {
            GenerateExp(expStmt->expression);
        }
    }

    static void
    GenerateFunction(const std::unique_ptr<FunctionDef>& func) {
        currentFunction = func->name;
        functionVariables[currentFunction].clear();
        stackSize = 0;

        bool isMainFunction = (func->name == "main");

        EmitLine("_" + func->name + ":");

        EmitLine("\tstp x29, x30, [sp, #-16]!");
        EmitLine("\tmov x29, sp");

        int totalStackSize = ((func->allocationSize + 15) & ~15) + 16;
        if (totalStackSize > 16) {
            EmitLine("\tsub sp, sp, #" + std::to_string(totalStackSize - 16));
        }

        for (const auto& stmt : func->statements) {
            GenerateStatement(stmt);
        }

        if (totalStackSize > 16) {
            EmitLine("\tadd sp, sp, #" + std::to_string(totalStackSize - 16));
        }
        EmitLine("\tldp x29, x30, [sp], #16");

        if (isMainFunction) {
            EmitLine("\tmov x16, #1");
            EmitLine("\tsvc #0x80");
        } else {
            EmitLine("\tret");
        }
    }

public:
    static std::string
    GenerateAssembly(const std::unique_ptr<CompilationUnit>& program) {
        assembly.str("");
        assembly.clear();
        EmitLine("\t.globl _main");
        EmitLine("\t.align 4");

        for (auto& statement : program->statements) {
            if (dynamic_cast<FunctionDef*>(statement.get())) {
                GenerateFunction(std::unique_ptr<FunctionDef>(dynamic_cast<FunctionDef *>(statement.release())));
            } else {
                GenerateStatement(statement);
            }
        }
        return assembly.str();
    }
};

#endif // GENERATOR_CPP