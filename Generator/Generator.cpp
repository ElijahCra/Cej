//
// Created by Elijah on 8/13/2024.
// Updated on 3/8/2025
//

#ifndef GENERATOR_CPP
#define GENERATOR_CPP

#include <sstream>
#include <unordered_map>
#include <memory>
#include <stack>
#include "../Parser/ParserTypes.hpp"
#include "../Analyzer/SymbolTable.hpp"

class Generator {
public:
    static std::string
    GenerateAssembly(const std::unique_ptr<Program>& program) {
        assembly.str("");
        assembly.clear();
        Reset();

        // First pass: Register all global variables
        for (const auto& decl : program->declarations) {
            if (const auto* varDecl = dynamic_cast<const VariableDeclaration*>(decl.get())) {
                // Add to global variables map
                globalVariables[varDecl->name] = varDecl->name;
            }
        }

        // Data section first for global variables
        EmitLine("\t.data");
        EmitLine("\t.p2align 2");  // Align to 4-byte boundary

        // Generate all global variables
        for (const auto& decl : program->declarations) {
            if (const auto* varDecl = dynamic_cast<const VariableDeclaration*>(decl.get())) {
                if (currentFunction.empty()) { // If we're not inside a function
                    // Global variable definition
                    EmitLine("\t.globl _" + varDecl->name);
                    EmitLine("_" + varDecl->name + ":");

                    // Handle initialization
                    if (varDecl->initializer.has_value()) {
                        if (const auto* singleInit = dynamic_cast<const SingleInit*>(varDecl->initializer.value().get())) {
                            if (const auto* constant = dynamic_cast<const Constant*>(singleInit->expression.get())) {
                                // If it's a simple constant, emit it directly
                                int value = 0;
                                if (constant->type == ConstantType::Int) {
                                    value = std::get<int>(constant->value);
                                }
                                EmitLine("\t.long " + std::to_string(value));
                            } else {
                                EmitLine("\t.long 0");  // Default to 0 for complex initializers
                            }
                        } else {
                            EmitLine("\t.long 0");  // Default to 0 for complex initializers
                        }
                    } else {
                        EmitLine("\t.long 0");  // Default to 0 for uninitialized variables
                    }
                }
            }
        }

        // Text section for code
        EmitLine("\t.text");
        EmitLine("\t.p2align 3");  // Align to 8-byte boundary (2^3)

        // Generate code for function declarations
        for (const auto& decl : program->declarations) {
            if (const auto* funcDecl = dynamic_cast<const FunctionDeclaration*>(decl.get())) {
                GenerateFunctionDeclaration(*funcDecl);
            }
        }

        return assembly.str();
    }

private:
    inline static std::stringstream assembly;
    inline static std::unordered_map<std::string, std::unordered_map<std::string, int>> functionVariables;
    inline static std::unordered_map<std::string, std::unordered_map<std::string, int>> structOffsets;
    inline static std::unordered_map<std::string, std::string> globalVariables; // Track global variables
    inline static int stackSize;
    inline static int labelCounter;
    inline static std::string currentFunction;

    // Scope management for control structures
    inline static std::stack<int> scopeStack;
    inline static int breakLabelId;
    inline static int continueLabelId;

    static void Reset() {
        functionVariables.clear();
        structOffsets.clear();
        globalVariables.clear();
        stackSize = 0;
        labelCounter = 0;
        currentFunction = "";
        while (!scopeStack.empty()) scopeStack.pop();
        breakLabelId = -1;
        continueLabelId = -1;
    }

    static void
    EmitLine(const std::string& line) {
        assembly << line << "\n";
    }

    static int
    GetNextLabel() {
        return labelCounter++;
    }

    static std::string
    GenerateLabel(const std::string& prefix) {
        return ".L" + prefix + std::to_string(GetNextLabel());
    }

    static void
    AllocateVariable(const std::string& name, int size = 8) {
        // Align to 8-byte boundary
        size = (size + 7) & ~7;
        stackSize += size;
        functionVariables[currentFunction][name] = -stackSize;
    }

    static std::pair<bool, int>
    GetVariableOffset(const std::string& name) {
        // First check if it's a local variable
        auto it = functionVariables[currentFunction].find(name);
        if (it != functionVariables[currentFunction].end()) {
            return {false, it->second}; // false = local variable (not global)
        }

        // Check if it's a global variable
        auto globalIt = globalVariables.find(name);
        if (globalIt != globalVariables.end()) {
            return {true, 0}; // true = global variable
        }

        throw std::runtime_error("Variable not found: " + name);
    }

    // Type size calculation
    static int
    GetTypeSize(const Type& type) {
        if (const auto* primType = dynamic_cast<const PrimitiveTypeNode*>(&type)) {
            switch (primType->type) {
                case PrimitiveType::Char: return 1;
                case PrimitiveType::Int: return 4;
                case PrimitiveType::Long: return 8;
                case PrimitiveType::Float: return 4;
                case PrimitiveType::Double: return 8;
                case PrimitiveType::Void: return 0;
                default: return 8; // Default size
            }
        } else if (const auto* ptrType = dynamic_cast<const PointerType*>(&type)) {
            return 8; // All pointers are 8 bytes on ARM64
        } else if (const auto* arrayType = dynamic_cast<const ArrayType*>(&type)) {
            return GetTypeSize(*arrayType->elementType) * arrayType->size;
        } else if (const auto* structType = dynamic_cast<const StructType*>(&type)) {
            // Would need struct size lookup - simplified here
            return 8; // Default to pointer size for structs
        } else if (const auto* funType = dynamic_cast<const FunType*>(&type)) {
            return 8; // Function pointers are 8 bytes
        }
        return 8; // Default to 8 bytes
    }

    static void
    GenerateDeclaration(Declaration& decl) {
        if (const auto* funcDecl = dynamic_cast<FunctionDeclaration*>(&decl)) {
            GenerateFunctionDeclaration(*funcDecl);
        } else if (const auto* structDecl = dynamic_cast<StructDeclaration*>(&decl)) {
            GenerateStructDeclaration(*structDecl);
        }
        // Variable declarations are handled directly in GenerateAssembly
    }

    static void
    GenerateVariableDeclaration(const VariableDeclaration& varDecl) {
        // For global variables - not in a function context
        if (currentFunction.empty()) {
            // Record this as a global variable
            globalVariables[varDecl.name] = varDecl.name;

            // Switch to data section - use standard syntax for maximum compatibility
            EmitLine("\t.data");
            EmitLine("\t.p2align 2");  // Align to 4-byte boundary (2^2)
            EmitLine("\t.globl _" + varDecl.name);
            EmitLine("_" + varDecl.name + ":");

            // Handle initialization
            if (varDecl.initializer.has_value()) {
                if (const auto* singleInit = dynamic_cast<const SingleInit*>(varDecl.initializer.value().get())) {
                    if (const auto* constant = dynamic_cast<const Constant*>(singleInit->expression.get())) {
                        // If it's a simple constant, emit it directly
                        int value = 0;
                        if (constant->type == ConstantType::Int) {
                            value = std::get<int>(constant->value);
                        }
                        EmitLine("\t.long " + std::to_string(value));
                    } else {
                        EmitLine("\t.long 0");  // Default to 0 for complex initializers
                    }
                } else {
                    EmitLine("\t.long 0");  // Default to 0 for complex initializers
                }
            } else {
                EmitLine("\t.long 0");  // Default to 0 for uninitialized variables
            }

            // Switch back to text section
            EmitLine("\t.text");
        } else {
            // Local variables are handled in statement generation
        }
    }

    static void
    GenerateStructDeclaration(const StructDeclaration& structDecl) {
        // Store structure member offsets for later usage
        int offset = 0;
        for (const auto& member : structDecl.members) {
            structOffsets[structDecl.tag][member->memberName] = offset;
            offset += GetTypeSize(*member->memberType);
            // Align to 8-byte boundary for each member
            offset = (offset + 7) & ~7;
        }
    }

    static void
    GenerateFunctionDeclaration(const FunctionDeclaration& funcDecl) {
        currentFunction = funcDecl.name;
        functionVariables[currentFunction].clear();
        stackSize = 0;

        bool isMainFunction = (funcDecl.name == "main");

        // Function entry
        EmitLine("\t.globl _" + funcDecl.name);
        EmitLine("_" + funcDecl.name + ":");

        // Function prologue
        EmitLine("\tstp x29, x30, [sp, #-16]!"); // Save frame pointer and link register
        EmitLine("\tmov x29, sp");               // Set up frame pointer

        // Allocate space for parameters
        for (size_t i = 0; i < funcDecl.parameters.size(); i++) {
            const auto& param = funcDecl.parameters[i];
            AllocateVariable(param->name, GetTypeSize(*param->paramType));

            // Move parameter from register to stack
            if (i < 8) { // First 8 parameters are in registers x0-x7
                EmitLine("\tstr w" + std::to_string(i) + ", [x29, #" +
                        std::to_string(functionVariables[currentFunction][param->name]) + "]");
            } else {
                // Parameters beyond the first 8 are passed on the stack
                // Not implemented here
            }
        }

        // Reserve space for all local variables upfront
        if (funcDecl.body.has_value()) {
            // Pre-scan for variable declarations to get total stack size
            PreScanForVariables(*funcDecl.body.value());

            // Allocate the stack space all at once
            int totalStackSize = (stackSize + 15) & ~15; // Align to 16 bytes
            if (totalStackSize > 0) {
                EmitLine("\tsub sp, sp, #" + std::to_string(totalStackSize));
            }

            // Generate the actual code
            GenerateBlock(*funcDecl.body.value());
        }

        // Function epilogue
        EmitLine(".L" + funcDecl.name + "_exit:");

        // Restore stack pointer back to frame pointer minus 16 (for saved fp and lr)
        EmitLine("\tmov sp, x29");
        EmitLine("\tldp x29, x30, [sp], #16");  // Restore frame pointer and link register

        if (isMainFunction) {
            // Special exit sequence for main
            EmitLine("\tmov x0, #0");           // Return value 0
            EmitLine("\tmov x16, #1");          // System call number for exit
            EmitLine("\tsvc #0x80");            // Supervisor call
        } else {
            EmitLine("\tret");                  // Return
        }
    }

    static void
    GenerateBlock(const Block& block) {
        scopeStack.push(GetNextLabel());

        for (const auto& item : block.items) {
            if (const auto* stmtItem = dynamic_cast<const BlockItemStatement*>(item.get())) {
                GenerateStatement(*stmtItem->statement);
            } else if (const auto* declItem = dynamic_cast<const BlockItemDeclaration*>(item.get())) {
                if (const auto* varDecl = dynamic_cast<const VariableDeclaration*>(declItem->declaration.get())) {
                    // Variable already allocated in pre-scan, just need to handle initialization
                    if (varDecl->initializer.has_value()) {
                        GenerateInitializer(*varDecl->initializer.value(),
                                          functionVariables[currentFunction][varDecl->name]);
                    }
                } else {
                    GenerateDeclaration(*declItem->declaration);
                }
            }
        }

        scopeStack.pop();
    }

    static void
    GenerateInitializer(const Initializer& init, int offset) {
        if (const auto* singleInit = dynamic_cast<const SingleInit*>(&init)) {
            GenerateExpression(*singleInit->expression);
            EmitLine("\tstr w0, [x29, #" + std::to_string(offset) + "]");
        } else if (const auto* compoundInit = dynamic_cast<const CompoundInit*>(&init)) {
            // Compound initializer (e.g., array or struct)
            int currentOffset = offset;
            for (const auto& subInit : compoundInit->initializers) {
                GenerateInitializer(*subInit, currentOffset);
                currentOffset += 8; // Simplified - assumes each element is 8 bytes
            }
        }
    }

    static void
    GenerateStatement(const Statement& stmt) {
        if (const auto* returnStmt = dynamic_cast<const ReturnStatement*>(&stmt)) {
            if (returnStmt->expression.has_value()) {
                GenerateExpression(*returnStmt->expression.value());
                // Return value is already in x0
            }
            EmitLine("\tb .L" + currentFunction + "_exit");
        } else if (const auto* exprStmt = dynamic_cast<const ExpressionStatement*>(&stmt)) {
            GenerateExpression(*exprStmt->expression);
            // Result in x0 is discarded for expression statements
        } else if (const auto* compoundStmt = dynamic_cast<const CompoundStatement*>(&stmt)) {
            GenerateBlock(*compoundStmt->block);
        } else if (const auto* ifStmt = dynamic_cast<const IfStatement*>(&stmt)) {
            GenerateIfStatement(*ifStmt);
        } else if (const auto* whileStmt = dynamic_cast<const WhileStatement*>(&stmt)) {
            GenerateWhileStatement(*whileStmt);
        } else if (const auto* forStmt = dynamic_cast<const ForStatement*>(&stmt)) {
            GenerateForStatement(*forStmt);
        } else if (const auto* breakStmt = dynamic_cast<const BreakStatement*>(&stmt)) {
            if (breakLabelId != -1) {
                EmitLine("\tb .Lbreak" + std::to_string(breakLabelId));
            } else {
                throw std::runtime_error("Break statement outside of loop");
            }
        } else if (const auto* continueStmt = dynamic_cast<const ContinueStatement*>(&stmt)) {
            if (continueLabelId != -1) {
                EmitLine("\tb .Lcontinue" + std::to_string(continueLabelId));
            } else {
                throw std::runtime_error("Continue statement outside of loop");
            }
        } else if (const auto* doWhileStmt = dynamic_cast<const DoWhileStatement*>(&stmt)) {
            GenerateDoWhileStatement(*doWhileStmt);
        } else if (dynamic_cast<const NullStatement*>(&stmt)) {
            // Nothing to do for null statements
        }
    }

    static void
    GenerateIfStatement(const IfStatement& ifStmt) {
        int elseLabel = GetNextLabel();
        int endLabel = GetNextLabel();

        // Generate condition
        GenerateExpression(*ifStmt.condition);
        EmitLine("\tcmp x0, #0");
        EmitLine("\tbeq .Lelse" + std::to_string(elseLabel));

        // Generate then branch
        GenerateStatement(*ifStmt.thenBranch);
        EmitLine("\tb .Lendif" + std::to_string(endLabel));

        // Generate else branch if it exists
        EmitLine(".Lelse" + std::to_string(elseLabel) + ":");
        if (ifStmt.elseBranch.has_value()) {
            GenerateStatement(*ifStmt.elseBranch.value());
        }

        EmitLine(".Lendif" + std::to_string(endLabel) + ":");
    }

    static void
    GenerateWhileStatement(const WhileStatement& whileStmt) {
        int startLabel = GetNextLabel();
        int endLabel = GetNextLabel();
        int continueLabel = GetNextLabel();

        // Save previous break/continue labels
        int prevBreakLabel = breakLabelId;
        int prevContinueLabel = continueLabelId;

        // Set current break/continue labels
        breakLabelId = endLabel;
        continueLabelId = continueLabel;

        EmitLine(".Lwhile" + std::to_string(startLabel) + ":");
        EmitLine(".Lcontinue" + std::to_string(continueLabel) + ":");

        // Generate condition
        GenerateExpression(*whileStmt.condition);
        EmitLine("\tcmp x0, #0");
        EmitLine("\tbeq .Lbreak" + std::to_string(endLabel));

        // Generate loop body
        GenerateStatement(*whileStmt.body);

        // Jump back to condition
        EmitLine("\tb .Lwhile" + std::to_string(startLabel));

        // End of loop
        EmitLine(".Lbreak" + std::to_string(endLabel) + ":");

        // Restore previous break/continue labels
        breakLabelId = prevBreakLabel;
        continueLabelId = prevContinueLabel;
    }

    static void
    GenerateDoWhileStatement(const DoWhileStatement& doWhileStmt) {
        int startLabel = GetNextLabel();
        int endLabel = GetNextLabel();
        int continueLabel = GetNextLabel();

        // Save previous break/continue labels
        int prevBreakLabel = breakLabelId;
        int prevContinueLabel = continueLabelId;

        // Set current break/continue labels
        breakLabelId = endLabel;
        continueLabelId = continueLabel;

        EmitLine(".Ldowhile" + std::to_string(startLabel) + ":");

        // Generate loop body
        GenerateStatement(*doWhileStmt.body);

        EmitLine(".Lcontinue" + std::to_string(continueLabel) + ":");

        // Generate condition
        GenerateExpression(*doWhileStmt.condition);
        EmitLine("\tcmp x0, #0");
        EmitLine("\tbne .Ldowhile" + std::to_string(startLabel));

        // End of loop
        EmitLine(".Lbreak" + std::to_string(endLabel) + ":");

        // Restore previous break/continue labels
        breakLabelId = prevBreakLabel;
        continueLabelId = prevContinueLabel;
    }

    static void
    GenerateForStatement(const ForStatement& forStmt) {
        int initLabel = GetNextLabel();
        int condLabel = GetNextLabel();
        int postLabel = GetNextLabel();
        int endLabel = GetNextLabel();

        // Save previous break/continue labels
        int prevBreakLabel = breakLabelId;
        int prevContinueLabel = continueLabelId;

        // Set current break/continue labels
        breakLabelId = endLabel;
        continueLabelId = postLabel;

        // Generate initialization
        if (const auto* initDecl = dynamic_cast<const InitDecl*>(forStmt.init.get())) {
            AllocateVariable(initDecl->declaration->name, GetTypeSize(*initDecl->declaration->varType));
            if (initDecl->declaration->initializer.has_value()) {
                GenerateInitializer(*initDecl->declaration->initializer.value(),
                    GetVariableOffset(initDecl->declaration->name).second);
            }
        } else if (const auto* initExp = dynamic_cast<const InitExp*>(forStmt.init.get())) {
            if (initExp->expression.has_value()) {
                GenerateExpression(*initExp->expression.value());
            }
        }

        EmitLine("\tb .Lfor_cond" + std::to_string(condLabel));

        // Condition check
        EmitLine(".Lfor_cond" + std::to_string(condLabel) + ":");
        if (forStmt.condition.has_value()) {
            GenerateExpression(*forStmt.condition.value());
            EmitLine("\tcmp x0, #0");
            EmitLine("\tbeq .Lfor_end" + std::to_string(endLabel));
        }

        // Body
        GenerateStatement(*forStmt.body);

        // Post-expression
        EmitLine(".Lfor_post" + std::to_string(postLabel) + ":");
        if (forStmt.post.has_value()) {
            GenerateExpression(*forStmt.post.value());
        }

        // Jump back to condition
        EmitLine("\tb .Lfor_cond" + std::to_string(condLabel));

        // End of loop
        EmitLine(".Lfor_end" + std::to_string(endLabel) + ":");

        // Restore previous break/continue labels
        breakLabelId = prevBreakLabel;
        continueLabelId = prevContinueLabel;
    }

    static void
    GenerateExpression(const Exp& exp) {
      if (const auto* constant = dynamic_cast<const Constant*>(&exp)) {
        GenerateConstant(*constant);
      } else if (const auto* var = dynamic_cast<const Var*>(&exp)) {
        auto [isGlobal, offset] = GetVariableOffset(var->name);
        if (isGlobal) {
          // Global variable access using a safer approach with explicit label
          EmitLine("\tadrp x0, _" + var->name + "@PAGE");
          EmitLine("\tadd x0, x0, _" + var->name + "@PAGEOFF");
          EmitLine("\tldr w0, [x0]");
        } else {
          // Load local variable
          EmitLine("\tldr w0, [x29, #" + std::to_string(offset) + "]");
        }
      } else if (const auto* binOp = dynamic_cast<const BinOp*>(&exp)) {
        GenerateBinaryOperation(*binOp);
      } else if (const auto* unOp = dynamic_cast<const UnOp*>(&exp)) {
        GenerateUnaryOperation(*unOp);
      } else if (const auto* assign = dynamic_cast<const Assignment*>(&exp)) {
        GenerateAssignment(*assign);
      } else if (const auto* functionCall = dynamic_cast<const FunctionCall*>(&exp)) {
        GenerateFunctionCall(*functionCall);
      } else if (const auto* conditional = dynamic_cast<const Conditional*>(&exp)) {
        GenerateConditionalExpression(*conditional);
      } else if (const auto* dereference = dynamic_cast<const Dereference*>(&exp)) {
        GenerateDereference(*dereference);
      } else if (const auto* addrOf = dynamic_cast<const AddrOf*>(&exp)) {
        GenerateAddressOf(*addrOf);
      } else if (const auto* subscript = dynamic_cast<const Subscript*>(&exp)) {
        GenerateSubscript(*subscript);
      } else if (const auto* sizeOfExp = dynamic_cast<const SizeOfExp*>(&exp)) {
        GenerateSizeOfExpression(*sizeOfExp);
      } else if (const auto* sizeOfType = dynamic_cast<const SizeOfType*>(&exp)) {
        GenerateSizeOfType(*sizeOfType);
      } else if (const auto* dot = dynamic_cast<const Dot*>(&exp)) {
        GenerateDotOperator(*dot);
      } else if (const auto* arrow = dynamic_cast<const Arrow*>(&exp)) {
        GenerateArrowOperator(*arrow);
      } else if (const auto* cast = dynamic_cast<const Cast*>(&exp)) {
        GenerateCast(*cast);
      } else if (const auto* stringLiteral = dynamic_cast<const StringLiteral*>(&exp)) {
        GenerateStringLiteral(*stringLiteral);
      } else if (const auto* binOp = dynamic_cast<const BinOp*>(&exp)) {
        GenerateBinaryOperation(*binOp);
      } else if (const auto* unOp = dynamic_cast<const UnOp*>(&exp)) {
        GenerateUnaryOperation(*unOp);
      } else if (const auto* assign = dynamic_cast<const Assignment*>(&exp)) {
        GenerateAssignment(*assign);
      } else if (const auto* functionCall = dynamic_cast<const FunctionCall*>(&exp)) {
        GenerateFunctionCall(*functionCall);
      } else if (const auto* conditional = dynamic_cast<const Conditional*>(&exp)) {
        GenerateConditionalExpression(*conditional);
      } else if (const auto* dereference = dynamic_cast<const Dereference*>(&exp)) {
        GenerateDereference(*dereference);
      } else if (const auto* addrOf = dynamic_cast<const AddrOf*>(&exp)) {
        GenerateAddressOf(*addrOf);
      } else if (const auto* subscript = dynamic_cast<const Subscript*>(&exp)) {
        GenerateSubscript(*subscript);
      } else if (const auto* sizeOfExp = dynamic_cast<const SizeOfExp*>(&exp)) {
        GenerateSizeOfExpression(*sizeOfExp);
      } else if (const auto* sizeOfType = dynamic_cast<const SizeOfType*>(&exp)) {
        GenerateSizeOfType(*sizeOfType);
      } else if (const auto* dot = dynamic_cast<const Dot*>(&exp)) {
        GenerateDotOperator(*dot);
      } else if (const auto* arrow = dynamic_cast<const Arrow*>(&exp)) {
        GenerateArrowOperator(*arrow);
      } else if (const auto* cast = dynamic_cast<const Cast*>(&exp)) {
        GenerateCast(*cast);
      } else if (const auto* stringLiteral = dynamic_cast<const StringLiteral*>(&exp)) {
        GenerateStringLiteral(*stringLiteral);
      }
    }

    static void
    GenerateConstant(const Constant& constant) {
        switch (constant.type) {
            case ConstantType::Int:
            case ConstantType::Long: {
                int value = std::get<int>(constant.value);
                EmitLine("\tmov x0, #" + std::to_string(value));
                break;
            }
            case ConstantType::UInt:
            case ConstantType::ULong: {
                unsigned int value = std::get<unsigned int>(constant.value);
                EmitLine("\tmov x0, #" + std::to_string(value));
                break;
            }
            case ConstantType::Double: {
                // Loading floating point constants is complex on ARM
                // This is a simplified approach
                double value = std::get<double>(constant.value);
                std::string label = GenerateLabel("float_const");
                EmitLine("\tadrp x0, " + label + "@PAGE");
                EmitLine("\tadd x0, x0, " + label + "@PAGEOFF");
                EmitLine("\tldr d0, [x0]");
                // Also need to add the float constant to a data section
                // Not implemented here
                break;
            }
            case ConstantType::Char: {
                char value = std::get<char>(constant.value);
                EmitLine("\tmov w0, #" + std::to_string(static_cast<int>(value)));
                break;
            }
            case ConstantType::UChar: {
                unsigned char value = std::get<unsigned char>(constant.value);
                EmitLine("\tmov w0, #" + std::to_string(static_cast<unsigned int>(value)));
                break;
            }
        }
    }

    static void
    GenerateStringLiteral(const StringLiteral& stringLiteral) {
        std::string label = GenerateLabel("str");

        // Store the string in the .data section
        std::string currentSection = "\t.text";
        EmitLine("\t.data");
        EmitLine(label + ":");
        EmitLine("\t.asciz \"" + stringLiteral.value + "\"");
        EmitLine(currentSection);

        // Load the address of the string
        EmitLine("\tadrp x0, " + label + "@PAGE");
        EmitLine("\tadd x0, x0, " + label + "@PAGEOFF");
    }

    static void
    GenerateBinaryOperation(const BinOp& binOp) {
        // Generate right operand first and push to stack
        GenerateExpression(*binOp.rhs);
        EmitLine("\tstr x0, [sp, #-16]!");

        // Generate left operand
        GenerateExpression(*binOp.lhs);

        // Load right operand from stack to x1
        EmitLine("\tldr x1, [sp], #16");

        // Perform operation based on operator type
        switch (binOp.op) {
            case BinaryOperator::Add:
                EmitLine("\tadd x0, x0, x1");
                break;
            case BinaryOperator::Subtract:
                EmitLine("\tsub x0, x0, x1");
                break;
            case BinaryOperator::Multiply:
                EmitLine("\tmul x0, x0, x1");
                break;
            case BinaryOperator::Divide:
                EmitLine("\tsdiv x0, x0, x1");
                break;
            case BinaryOperator::Remainder:
                // Modulo operation requires additional steps
                EmitLine("\tsdiv x2, x0, x1");     // x2 = x0 / x1
                EmitLine("\tmul x2, x2, x1");      // x2 = x2 * x1
                EmitLine("\tsub x0, x0, x2");      // x0 = x0 - x2
                break;
            case BinaryOperator::And:
                EmitLine("\tands x0, x0, x1");     // Logical AND
                EmitLine("\tcset x0, ne");         // Set x0 to 1 if result is not zero
                break;
            case BinaryOperator::Or:
                EmitLine("\torr x0, x0, x1");      // Logical OR
                EmitLine("\tcmp x0, #0");          // Compare with zero
                EmitLine("\tcset x0, ne");         // Set x0 to 1 if result is not zero
                break;
            case BinaryOperator::Equal:
                EmitLine("\tcmp x0, x1");
                EmitLine("\tcset x0, eq");         // Set x0 to 1 if equal
                break;
            case BinaryOperator::NotEqual:
                EmitLine("\tcmp x0, x1");
                EmitLine("\tcset x0, ne");         // Set x0 to 1 if not equal
                break;
            case BinaryOperator::LessThan:
                EmitLine("\tcmp x0, x1");
                EmitLine("\tcset x0, lt");         // Set x0 to 1 if less than
                break;
            case BinaryOperator::LessOrEqual:
                EmitLine("\tcmp x0, x1");
                EmitLine("\tcset x0, le");         // Set x0 to 1 if less than or equal
                break;
            case BinaryOperator::GreaterThan:
                EmitLine("\tcmp x0, x1");
                EmitLine("\tcset x0, gt");         // Set x0 to 1 if greater than
                break;
            case BinaryOperator::GreaterOrEqual:
                EmitLine("\tcmp x0, x1");
                EmitLine("\tcset x0, ge");         // Set x0 to 1 if greater than or equal
                break;
        }
    }

    static void
    GenerateUnaryOperation(const UnOp& unOp) {
        GenerateExpression(*unOp.operand);

        switch (unOp.op) {
            case UnaryOperator::Negate:
                EmitLine("\tneg x0, x0");
                break;
            case UnaryOperator::Complement:
                EmitLine("\tmvn x0, x0");         // Bitwise NOT
                break;
            case UnaryOperator::Not:
                EmitLine("\tcmp x0, #0");
                EmitLine("\tcset x0, eq");        // Set x0 to 1 if zero
                break;
        }
    }

    static void
    GenerateAssignment(const Assignment& assign) {
        // First generate the right-hand side expression
        GenerateExpression(*assign.rhs);

        // Store the result back based on the type of left-hand side
        if (const auto* var = dynamic_cast<const Var*>(assign.lhs.get())) {
            // Simple variable assignment
            auto [isGlobal, offset] = GetVariableOffset(var->name);
            if (isGlobal) {
                // Save result in x1 temporarily
                EmitLine("\tmov w1, w0");

                // Load global variable address using PAGE-relative addressing
                EmitLine("\tadrp x0, _" + var->name + "@PAGE");
                EmitLine("\tadd x0, x0, _" + var->name + "@PAGEOFF");

                // Store value to global variable
                EmitLine("\tstr w1, [x0]");

                // Put result back in x0
                EmitLine("\tmov w0, w1");
            } else {
                EmitLine("\tstr w0, [x29, #" + std::to_string(offset) + "]");
            }
        } else if (const auto* dereference = dynamic_cast<const Dereference*>(assign.lhs.get())) {
            // Pointer dereference assignment: *ptr = value
            // Push value to stack
            EmitLine("\tstr x0, [sp, #-16]!");

            // Generate pointer expression
            GenerateExpression(*dereference->expression);

            // Pop value from stack to x1
            EmitLine("\tldr x1, [sp], #16");

            // Store value to the address in x0
            EmitLine("\tstr x1, [x0]");

            // Result of assignment is the value that was assigned
            EmitLine("\tmov x0, x1");
        } else if (const auto* subscript = dynamic_cast<const Subscript*>(assign.lhs.get())) {
            // Array assignment: arr[index] = value
            // Save value to stack
            EmitLine("\tstr x0, [sp, #-16]!");

            // Generate array base address
            GenerateExpression(*subscript->array);

            // Save array base to stack
            EmitLine("\tstr x0, [sp, #-16]!");

            // Generate index
            GenerateExpression(*subscript->index);

            // Multiply index by element size (assuming 8 bytes for simplicity)
            EmitLine("\tlsl x0, x0, #3");  // x0 = x0 * 8

            // Load array base from stack
            EmitLine("\tldr x1, [sp], #16");

            // Add offset to get element address
            EmitLine("\tadd x0, x1, x0");

            // Load value from stack
            EmitLine("\tldr x1, [sp], #16");

            // Store value to array element
            EmitLine("\tstr x1, [x0]");

            // Result of assignment is the value
            EmitLine("\tmov x0, x1");
        } else if (const auto* dot = dynamic_cast<const Dot*>(assign.lhs.get())) {
            // Struct member assignment: struct.member = value
            // Save value to stack
            EmitLine("\tstr x0, [sp, #-16]!");

            // Generate struct base address
            GenerateExpression(*dot->structure);

            // Need to add the member offset, but we don't have type info here
            // This is simplified and would need actual struct information
            // For now, just assuming a fixed offset (example: 8 bytes)
            EmitLine("\tadd x0, x0, #8");

            // Load value from stack
            EmitLine("\tldr x1, [sp], #16");

            // Store value to struct member
            EmitLine("\tstr x1, [x0]");

            // Result of assignment is the value
            EmitLine("\tmov x0, x1");
        } else if (const auto* arrow = dynamic_cast<const Arrow*>(assign.lhs.get())) {
            // Struct pointer member assignment: ptr->member = value
            // Save value to stack
            EmitLine("\tstr x0, [sp, #-16]!");

            // Generate pointer expression
            GenerateExpression(*arrow->pointer);

            // Need to add the member offset - simplified as with dot operator
            EmitLine("\tadd x0, x0, #8");

            // Load value from stack
            EmitLine("\tldr x1, [sp], #16");

            // Store value to struct member
            EmitLine("\tstr x1, [x0]");

            // Result of assignment is the value
            EmitLine("\tmov x0, x1");
        }
    }

    static void
    GenerateFunctionCall(const FunctionCall& functionCall) {
        // Calculate stack alignment
        int argumentCount = functionCall.arguments.size();
        bool needStackAdjustment = (argumentCount > 8) || (argumentCount % 2 != 0);

        // Adjust stack if needed for alignment
        if (needStackAdjustment) {
            EmitLine("\tsub sp, sp, #16");
        }

        // Generate arguments in reverse order
        for (int i = argumentCount - 1; i >= 0; i--) {
            GenerateExpression(*functionCall.arguments[i]);
            if (i < 8) {
                // First 8 arguments go to registers x0-x7
                if (i > 0) {
                    EmitLine("\tmov x" + std::to_string(i) + ", x0");
                }
            } else {
                // Additional arguments go to stack
                EmitLine("\tstr x0, [sp, #" + std::to_string((i - 8) * 8) + "]");
            }
        }

        // Call the function
        EmitLine("\tbl _" + functionCall.name);

        // Restore stack if needed
        if (needStackAdjustment) {
            EmitLine("\tadd sp, sp, #16");
        }

        // Result is already in x0
    }

    static void
    GenerateConditionalExpression(const Conditional& conditional) {
        int falseLabel = GetNextLabel();
        int endLabel = GetNextLabel();

        // Generate condition
        GenerateExpression(*conditional.condition);
        EmitLine("\tcmp x0, #0");
        EmitLine("\tbeq .Lcond_false" + std::to_string(falseLabel));

        // Generate true expression
        GenerateExpression(*conditional.trueExp);
        EmitLine("\tb .Lcond_end" + std::to_string(endLabel));

        // Generate false expression
        EmitLine(".Lcond_false" + std::to_string(falseLabel) + ":");
        GenerateExpression(*conditional.falseExp);

        EmitLine(".Lcond_end" + std::to_string(endLabel) + ":");
    }

    static void
    GenerateDereference(const Dereference& dereference) {
        GenerateExpression(*dereference.expression);
        EmitLine("\tldr x0, [x0]");
    }

    static void
    GenerateAddressOf(const AddrOf& addrOf) {
        if (const auto* var = dynamic_cast<const Var*>(addrOf.expression.get())) {
            int offset = GetVariableOffset(var->name).second;
            EmitLine("\tadd x0, x29, #" + std::to_string(offset));
        } else {
            // More complex address-of operations would need special handling
            throw std::runtime_error("Unsupported addressof operation");
        }
    }

    static void
    GenerateSubscript(const Subscript& subscript) {
        // Generate array base address
        GenerateExpression(*subscript.array);

        // Save array base to stack
        EmitLine("\tstr x0, [sp, #-16]!");

        // Generate index
        GenerateExpression(*subscript.index);

        // Multiply index by element size (assuming 8 bytes for simplicity)
        EmitLine("\tlsl x0, x0, #3");  // x0 = x0 * 8

        // Load array base from stack
        EmitLine("\tldr x1, [sp], #16");

        // Add offset to get element address
        EmitLine("\tadd x0, x1, x0");

        // Load value from array element
        EmitLine("\tldr x0, [x0]");
    }

    static void
    GenerateSizeOfExpression(const SizeOfExp& sizeOfExp) {
        // For sizeof expression, we'd need type information
        // This is a simplified version that returns 8 (typical size of a pointer/long)
        EmitLine("\tmov x0, #8");
    }

    static void
    GenerateSizeOfType(const SizeOfType& sizeOfType) {
        int size = GetTypeSize(*sizeOfType.type);
        EmitLine("\tmov x0, #" + std::to_string(size));
    }

    static void
    GenerateDotOperator(const Dot& dot) {
        // Generate struct base address
        GenerateExpression(*dot.structure);

        // Need struct type information to calculate member offset
        // This is a simplified version assuming member offset is 8
        EmitLine("\tadd x0, x0, #8");

        // Load value from struct member
        EmitLine("\tldr x0, [x0]");
    }

    static void
    GenerateArrowOperator(const Arrow& arrow) {
        // Generate pointer to struct
        GenerateExpression(*arrow.pointer);

        // Need struct type information to calculate member offset
        // This is a simplified version assuming member offset is 8
        EmitLine("\tadd x0, x0, #8");

        // Load value from struct member
        EmitLine("\tldr x0, [x0]");
    }

    static void
    GenerateCast(const Cast& cast) {
        // Generate the expression to be cast
        GenerateExpression(*cast.expression);

        // Handle cast based on type
        // This is simplified and would need more complex handling for different type combinations
        if (const auto* primType = dynamic_cast<const PrimitiveTypeNode*>(cast.targetType.get())) {
            switch (primType->type) {
                case PrimitiveType::Char:
                    EmitLine("\tand x0, x0, #0xFF");  // Mask to keep only the lowest byte
                    break;
                case PrimitiveType::Int:
                    EmitLine("\tsxtw x0, w0");  // Sign-extend word to xword
                    break;
                case PrimitiveType::Float:
                case PrimitiveType::Double:
                    // Floating point conversion is complex
                    // Not implemented here
                    break;
                case PrimitiveType::Long:
                    // Already 64-bit
                    break;
                case PrimitiveType::Void:
                    // No operation needed for void cast
                    break;
            }
        }
    }
  static void
    PreScanForVariables(const Block& block) {
      for (const auto& item : block.items) {
        if (const auto* declItem = dynamic_cast<const BlockItemDeclaration*>(item.get())) {
          if (const auto* varDecl = dynamic_cast<const VariableDeclaration*>(declItem->declaration.get())) {
            // Record variable without generating code
            AllocateVariable(varDecl->name, GetTypeSize(*varDecl->varType));
          }
        } else if (const auto* stmtItem = dynamic_cast<const BlockItemStatement*>(item.get())) {
          if (const auto* compoundStmt = dynamic_cast<const CompoundStatement*>(stmtItem->statement.get())) {
            // Recursively scan nested blocks
            PreScanForVariables(*compoundStmt->block);
          }
        }
      }
    }
};

#endif // GENERATOR_CPP