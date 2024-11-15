// SemanticAnalyzer.hpp
#ifndef SEMANTICANALYZER_HPP
#define SEMANTICANALYZER_HPP

#include <stdexcept>
#include "../Parser/ParserTypes.hpp"
#include "SymbolTable.hpp"

class SemanticAnalyzer {
public:
    SemanticAnalyzer() = default;

    void analyze(Program& program) {
        symbolTable.enterScope();
        for (auto& decl : program.declarations) {
            analyzeDeclaration(*decl);
        }
        symbolTable.exitScope();
    }

private:
    SymbolTable symbolTable;

    void analyzeDeclaration(Declaration& decl) {
        if (auto varDecl = dynamic_cast<VariableDeclaration*>(&decl)) {
            analyzeVariableDeclaration(*varDecl);
        } else if (auto funcDecl = dynamic_cast<FunctionDeclaration*>(&decl)) {
            analyzeFunctionDeclaration(*funcDecl);
        } else if (auto structDecl = dynamic_cast<StructDeclaration*>(&decl)) {
            analyzeStructDeclaration(*structDecl);
        } else {
            // Handle other declaration types
        }
    }

    void analyzeVariableDeclaration(VariableDeclaration& varDecl) {
        // Check if variable is already declared in current scope
        if (!symbolTable.insert(varDecl.name, Symbol(varDecl.name, varDecl.varType->clone(), Symbol::Kind::Variable))) {
            throw std::runtime_error("Variable '" + varDecl.name + "' is already declared in this scope.");
        }

        // Optionally, analyze the initializer
        if (varDecl.initializer.has_value()) {
            analyzeInitializer(*varDecl.initializer.value());
        }
    }

    void analyzeInitializer(Initializer& init) {
        if (auto singleInit = dynamic_cast<SingleInit*>(&init)) {
            analyzeExpression(*singleInit->expression);
        } else if (auto compoundInit = dynamic_cast<CompoundInit*>(&init)) {
            for (auto& subInit : compoundInit->initializers) {
                analyzeInitializer(*subInit);
            }
        }
    }

    void analyzeFunctionDeclaration(FunctionDeclaration& funcDecl) {
        // Build function type
        std::vector<std::unique_ptr<Type>> paramTypes;
        for (auto& param : funcDecl.parameters) {
            paramTypes.push_back(param->paramType->clone());
        }
        auto funType = std::make_unique<FunType>(std::move(paramTypes), funcDecl.funType->clone());

        // Check if function is already declared
        if (!symbolTable.insert(funcDecl.name, Symbol(funcDecl.name, std::move(funType), Symbol::Kind::Function))) {
            throw std::runtime_error("Function '" + funcDecl.name + "' is already declared.");
        }

        symbolTable.enterScope(); // Enter function scope

        // Add parameters to symbol table
        for (auto& param : funcDecl.parameters) {
            if (!symbolTable.insert(param->name, Symbol(param->name, param->paramType->clone(), Symbol::Kind::Variable))) {
                throw std::runtime_error("Parameter '" + param->name + "' is already declared.");
            }
        }

        // Analyze function body
        if (funcDecl.body.has_value()) {
            analyzeBlock(*funcDecl.body.value());
        }

        symbolTable.exitScope(); // Exit function scope
    }

    void analyzeStructDeclaration(StructDeclaration& structDecl) {
        // Store the struct type in the symbol table
        auto structType = std::make_unique<StructType>(structDecl.tag);

        if (!symbolTable.insert(structDecl.tag, Symbol(structDecl.tag, std::move(structType), Symbol::Kind::Struct))) {
            throw std::runtime_error("Struct '" + structDecl.tag + "' is already declared.");
        }

        // Optionally, analyze members
        // For simplicity, we're not adding struct members to the symbol table
    }

    void analyzeBlock(Block& block) {
        symbolTable.enterScope();
        for (auto& item : block.items) {
            analyzeBlockItem(*item);
        }
        symbolTable.exitScope();
    }

    void analyzeBlockItem(BlockItem& item) {
        if (auto stmtItem = dynamic_cast<BlockItemStatement*>(&item)) {
            analyzeStatement(*stmtItem->statement);
        } else if (auto declItem = dynamic_cast<BlockItemDeclaration*>(&item)) {
            analyzeDeclaration(*declItem->declaration);
        }
    }

    void analyzeStatement(Statement& stmt) {
        if (auto exprStmt = dynamic_cast<ExpressionStatement*>(&stmt)) {
            analyzeExpression(*exprStmt->expression);
        } else if (auto returnStmt = dynamic_cast<ReturnStatement*>(&stmt)) {
            if (returnStmt->expression.has_value()) {
                analyzeExpression(*returnStmt->expression.value());
            }
        } else if (auto compoundStmt = dynamic_cast<CompoundStatement*>(&stmt)) {
            analyzeBlock(*compoundStmt->block);
        } else if (auto ifStmt = dynamic_cast<IfStatement*>(&stmt)) {
            analyzeExpression(*ifStmt->condition);
            analyzeStatement(*ifStmt->thenBranch);
            if (ifStmt->elseBranch.has_value()) {
                analyzeStatement(*ifStmt->elseBranch.value());
            }
        } else if (auto whileStmt = dynamic_cast<WhileStatement*>(&stmt)) {
            analyzeExpression(*whileStmt->condition);
            analyzeStatement(*whileStmt->body);
        } else if (auto forStmt = dynamic_cast<ForStatement*>(&stmt)) {
            symbolTable.enterScope();
            if (auto initDecl = dynamic_cast<InitDecl*>(forStmt->init.get())) {
                analyzeVariableDeclaration(*initDecl->declaration);
            } else if (auto initExp = dynamic_cast<InitExp*>(forStmt->init.get())) {
                if (initExp->expression.has_value()) {
                    analyzeExpression(*initExp->expression.value());
                }
            }
            if (forStmt->condition.has_value()) {
                analyzeExpression(*forStmt->condition.value());
            }
            if (forStmt->post.has_value()) {
                analyzeExpression(*forStmt->post.value());
            }
            analyzeStatement(*forStmt->body);
            symbolTable.exitScope();
        }
        // Handle other statement types as needed
    }

    void analyzeExpression(Exp& exp) {
        if (auto varExp = dynamic_cast<Var*>(&exp)) {
            // Check if variable is declared
            auto symbol = symbolTable.lookup(varExp->name);
            if (!symbol) {
                throw std::runtime_error("Use of undeclared variable '" + varExp->name + "'.");
            }
        } else if (auto binOpExp = dynamic_cast<BinOp*>(&exp)) {
            analyzeExpression(*binOpExp->lhs);
            analyzeExpression(*binOpExp->rhs);
        } else if (auto unOpExp = dynamic_cast<UnOp*>(&exp)) {
            analyzeExpression(*unOpExp->operand);
        } else if (auto assignExp = dynamic_cast<Assignment*>(&exp)) {
            analyzeExpression(*assignExp->lhs);
            analyzeExpression(*assignExp->rhs);
        } else if (auto funcCallExp = dynamic_cast<FunctionCall*>(&exp)) {
            // Check if function is declared
            auto symbol = symbolTable.lookup(funcCallExp->name);
            if (!symbol || symbol->kind != Symbol::Kind::Function) {
                throw std::runtime_error("Call to undeclared function '" + funcCallExp->name + "'.");
            }
            // Analyze arguments
            for (auto& arg : funcCallExp->arguments) {
                analyzeExpression(*arg);
            }
        }
        // Handle other expression types as needed
    }
};

#endif // SEMANTICANALYZER_HPP
