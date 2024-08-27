//
// Created by Elijah on 8/13/2024.
//

#ifndef GENERATOR_CPP
#define GENERATOR_CPP

#include <sstream>
#include <unordered_map>
#include "Parser.cpp"

class Generator {
    inline static std::stringstream assembly;
    inline static std::unordered_map<std::string, int> variables;
    inline static int stackSize;
    inline static int labelCounter;

    static void EmitLine(const std::string& line) {
        assembly << line << "\n";
    }

    static void AllocateVariable(const std::string& name) {
        variables[name] = stackSize;
        stackSize += 8;
    }

    static void GenerateExp(const std::unique_ptr<Exp>& exp, bool useX1 = false) {
        std::string reg = useX1 ? "x1" : "x0";

        if (auto constant = dynamic_cast<Constant*>(exp.get())) {
            EmitLine("\tmov " + reg + ", #" + std::to_string(constant->value));
        } else if (auto var = dynamic_cast<Var*>(exp.get())) {
            int offset = variables[var->name];
            EmitLine("\tldr " + reg + ", [sp, #" + std::to_string(offset) + "]");
        } else if (auto binOp = dynamic_cast<BinOp*>(exp.get())) {
            if (auto rightConstant = dynamic_cast<Constant*>(binOp->rhs.get())) {
                GenerateExp(binOp->lhs);
                switch (binOp->op) {
                    case BinaryOperator::Add:
                        EmitLine("\tadd x0, x0, #" + std::to_string(rightConstant->value));
                        break;
                    case BinaryOperator::Sub:
                        EmitLine("\tsub x0, x0, #" + std::to_string(rightConstant->value));
                        break;
                    case BinaryOperator::Mul:
                        EmitLine("\tmov x1, #" + std::to_string(rightConstant->value));
                        EmitLine("\tmul x0, x0, x1");
                        break;
                    case BinaryOperator::Div:
                        EmitLine("\tmov x1, #" + std::to_string(rightConstant->value));
                        EmitLine("\tsdiv x0, x0, x1");
                        break;
                }
            } else {
                GenerateExp(binOp->rhs, true);
                GenerateExp(binOp->lhs);
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
            }
        } else if (auto unOp = dynamic_cast<UnOp*>(exp.get())) {
            GenerateExp(unOp->operand);
            if (unOp->op == UnaryOperator::Neg) {
                EmitLine("\tneg x0, x0");
            }
        } else if (auto assign = dynamic_cast<Assign*>(exp.get())) {
            GenerateExp(assign->value);
            int offset = variables[assign->name];
            EmitLine("\tstr x0, [sp, #" + std::to_string(offset) + "]");
        }
    }

    static void GenerateStatement(const std::unique_ptr<Statement>& stmt) {
        if (auto returnStmt = dynamic_cast<Return*>(stmt.get())) {
            GenerateExp(returnStmt->expression);
        } else if (auto declareStmt = dynamic_cast<Declare*>(stmt.get())) {
            AllocateVariable(declareStmt->name);
            if (declareStmt->initializer) {
                GenerateExp(*declareStmt->initializer);
                int offset = variables[declareStmt->name];
                EmitLine("\tstr x0, [sp, #" + std::to_string(offset) + "]");
            }
        } else if (auto expStmt = dynamic_cast<ExpStatement*>(stmt.get())) {
            GenerateExp(expStmt->expression);
        }
    }

    static void GenerateFunction(const std::unique_ptr<Function>& func) {
        variables.clear();
        stackSize = 0;
        labelCounter = 0;

        EmitLine("\t.globl _" + func->name);
        EmitLine("\t.align 4");
        EmitLine("_" + func->name + ":");

        EmitLine("\tstp x29, x30, [sp, #-16]!");
        EmitLine("\tmov x29, sp");

        if (func->allocationSize % 16 != 0) {
            func->allocationSize += 8;
        }

        EmitLine("\tsub sp, sp, #" + std::to_string(func->allocationSize));

        for (const auto& stmt : func->statements) {
            GenerateStatement(stmt);
        }

        if (stackSize % 16 != 0) {
            stackSize += 16 - (stackSize % 16);
        }

        if (func->allocationSize > 0) {
            EmitLine("\tadd sp, sp, #" + std::to_string(func->allocationSize));
        }

        EmitLine("\tldp x29, x30, [sp], #16");
        EmitLine("\tmov x16, #1");
        EmitLine("\tsvc #0x80");
    }

public:
    static std::string GenerateAssembly(const std::unique_ptr<Program>& program) {
        assembly.str("");
        assembly.clear();

        GenerateFunction(program->function);

        return assembly.str();
    }
};

#endif // GENERATOR_CPP