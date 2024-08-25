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

    static std::string CreateLabel() {
        return ".L" + std::to_string(labelCounter++);
    }

    static void AllocateVariable(const std::string& name) {
        variables[name] = stackSize;
        stackSize += 16;  // Allocate 16 bytes for each variable
    }

    static void GenerateExp(const std::unique_ptr<Exp>& exp) {
        if (auto constant = dynamic_cast<Constant*>(exp.get())) {
            EmitLine("\tmov x0, #" + std::to_string(constant->value));
        } else if (auto var = dynamic_cast<Var*>(exp.get())) {
            int offset = variables[var->name];
            EmitLine("\tldr x0, [sp, #" + std::to_string(offset) + "]");
        } else if (auto binOp = dynamic_cast<BinOp*>(exp.get())) {
            GenerateExp(binOp->right);
            EmitLine("\tstr x0, [sp, #-16]!");
            GenerateExp(binOp->left);
            EmitLine("\tldr x1, [sp], #16");
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
            int offset = variables[assign->name];
            EmitLine("\tstr x0, [sp, #" + std::to_string(offset) + "]");
        }
    }

    static void GenerateStatement(const std::unique_ptr<Statement>& stmt) {
        if (auto returnStmt = dynamic_cast<Return*>(stmt.get())) {
            GenerateExp(returnStmt->expression);
            EmitLine("\tb .Lreturn");
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

        EmitLine("\t.globl _" + func->name);
        EmitLine("\t.align 4");
        EmitLine("_" + func->name + ":");

        // Prologue
        EmitLine("\tstp x29, x30, [sp, #-16]!");
        EmitLine("\tmov x29, sp");

        // Generate code for each statement
        for (const auto& stmt : func->statements) {
            GenerateStatement(stmt);
        }

        // Epilogue
        EmitLine(".Lreturn:");
        EmitLine("\tldp x29, x30, [sp], #16");
        EmitLine("\tret");
    }

public:
    static std::string GenerateAssembly(const std::unique_ptr<Program>& program) {
        assembly.str("");
        assembly.clear();
        labelCounter = 0;

        GenerateFunction(program->function);

        return assembly.str();
    }
};

#endif // GENERATOR_CPP