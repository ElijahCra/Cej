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
    inline static int nextRegister;

    static void EmitLine(const std::string& line) {
        assembly << line << "\n";
    }

    static std::string CreateLabel() {
        return ".L" + std::to_string(labelCounter++);
    }

    static void AllocateVariable(const std::string& name) {
        variables[name] = stackSize;
        stackSize += 4;  // Allocate 4 bytes for each integer variable
    }


    static std::string GetNextRegister() {
        return "w" + std::to_string(nextRegister++);
    }

    static void ResetRegisters() {
        nextRegister = 0;
    }

    static void GenerateExp(const std::unique_ptr<Exp>& exp) {
        if (auto constant = dynamic_cast<Constant*>(exp.get())) {
            std::string reg = GetNextRegister();
            EmitLine("\tmov " + reg + ", #" + std::to_string(constant->value));
        } else if (auto var = dynamic_cast<Var*>(exp.get())) {
            int offset = variables[var->name];
            std::string reg = GetNextRegister();
            EmitLine("\tldr " + reg + ", [sp, #" + std::to_string(offset) + "]");
        } else if (auto binOp = dynamic_cast<BinOp*>(exp.get())) {
            GenerateExp(binOp->left);
            std::string leftReg = "w" + std::to_string(nextRegister - 1);
            GenerateExp(binOp->right);
            std::string rightReg = "w" + std::to_string(nextRegister - 1);
            std::string resultReg = GetNextRegister();
            switch (binOp->op) {
                case BinaryOperator::Add:
                    EmitLine("\tadd " + resultReg + ", " + leftReg + ", " + rightReg);
                    break;
                case BinaryOperator::Sub:
                    EmitLine("\tsub " + resultReg + ", " + leftReg + ", " + rightReg);
                    break;
                case BinaryOperator::Mul:
                    EmitLine("\tmul " + resultReg + ", " + leftReg + ", " + rightReg);
                    break;
                case BinaryOperator::Div:
                    EmitLine("\tsdiv " + resultReg + ", " + leftReg + ", " + rightReg);
                    break;
            }
        } else if (auto unOp = dynamic_cast<UnOp*>(exp.get())) {
            GenerateExp(unOp->operand);
            std::string operandReg = "w" + std::to_string(nextRegister - 1);
            std::string resultReg = GetNextRegister();
            if (unOp->op == UnaryOperator::Neg) {
                EmitLine("\tneg " + resultReg + ", " + operandReg);
            }
        } else if (auto assign = dynamic_cast<Assign*>(exp.get())) {
            GenerateExp(assign->value);
            std::string valueReg = "w" + std::to_string(nextRegister - 1);
            int offset = variables[assign->name];
            EmitLine("\tstr " + valueReg + ", [sp, #" + std::to_string(offset) + "]");
        }
    }

    static void GenerateStatement(const std::unique_ptr<Statement>& stmt) {
        ResetRegisters();
        if (auto returnStmt = dynamic_cast<Return*>(stmt.get())) {
            GenerateExp(returnStmt->expression);
            EmitLine("\tmov w0, w" + std::to_string(nextRegister - 1));
            EmitLine("\tb .Lreturn");
        } else if (auto declareStmt = dynamic_cast<Declare*>(stmt.get())) {
            AllocateVariable(declareStmt->name);
            if (declareStmt->initializer) {
                GenerateExp(*declareStmt->initializer);
                std::string valueReg = "w" + std::to_string(nextRegister - 1);
                int offset = variables[declareStmt->name];
                EmitLine("\tstr " + valueReg + ", [sp, #" + std::to_string(offset) + "]");
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

        // Prologue
        EmitLine("\tstp x29, x30, [sp, #-16]!");
        EmitLine("\tmov x29, sp");

        //Allocate variable space
        EmitLine("\t sub sp, sp, #" + std::to_string(func->allocationSize));

        // Generate code for each statement
        for (const auto& stmt : func->statements) {
            GenerateStatement(stmt);
        }

        // Align stack size to 16 bytes
        if (stackSize % 16 != 0) {
            stackSize += 16 - (stackSize % 16);
        }

        // Allocate stack space for local variables
        if (stackSize > 0) {
            EmitLine("\tsub sp, sp, #" + std::to_string(stackSize));
        }

        // Epilogue
        EmitLine(".Lreturn:");
        if (stackSize > 0) {
            EmitLine("\tadd sp, sp, #" + std::to_string(stackSize));
        }
        EmitLine("\tldp x29, x30, [sp], #16");
        EmitLine("\tret");
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