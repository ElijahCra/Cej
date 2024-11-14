//
// Created by Elijah Crain on 11/13/24.
//

// SemanticAnalyzer.hpp
#ifndef SEMANTICANALYZER_HPP
#define SEMANTICANALYZER_HPP

#include <iostream>
#include "../Parser/ParserTypes.hpp"
#include "SymbolTable.hpp"

class SemanticAnalyzer {
private:
    SymbolTable* currentScope;

public:
    SemanticAnalyzer() : currentScope(new SymbolTable()) {}

    void analyze(Program* program) {
        visitProgram(program);
    }

private:
    void enterScope() {
        currentScope = new SymbolTable(currentScope);
    }

    void leaveScope() {
        SymbolTable* oldScope = currentScope;
        currentScope = currentScope->getParent();
        delete oldScope;
    }

    void visitProgram(Program* program) {
        for (auto& decl : program->declarations) {
            visitDeclaration(decl.get());
        }
    }

    void visitDeclaration(Declaration* decl) {
        if (auto varDecl = dynamic_cast<VariableDeclaration*>(decl)) {
            visitVariableDeclaration(varDecl);
        } else if (auto funcDecl = dynamic_cast<FunctionDeclaration*>(decl)) {
            visitFunctionDeclaration(funcDecl);
        } else if (auto structDecl = dynamic_cast<StructDeclaration*>(decl)) {
            visitStructDeclaration(structDecl);
        }
    }

    void visitVariableDeclaration(VariableDeclaration* varDecl) {
        Symbol symbol(varDecl->name, varDecl->varType, SymbolKind::Variable);
        if (!currentScope->declare(varDecl->name, std::move(symbol))) {
            reportError("Variable '" + varDecl->name + "' redeclared");
        }

        if (varDecl->initializer.has_value()) {
            visitInitializer(varDecl->initializer.value().get());
        }
    }

    void visitFunctionDeclaration(FunctionDeclaration* funcDecl) {
        Symbol symbol(funcDecl->name, funcDecl->funType->clone(), SymbolKind::Function);
        if (!currentScope->declare(funcDecl->name, std::move(symbol))) {
            reportError("Function '" + funcDecl->name + "' redeclared");
        }

        enterScope();

        for (auto& param : funcDecl->parameters) {
            Symbol paramSymbol(param->name, param->paramType->clone(), SymbolKind::Variable);
            if (!currentScope->declare(param->name, std::move(paramSymbol))) {
                reportError("Parameter '" + param->name + "' redeclared");
            }
        }

        if (funcDecl->body.has_value()) {
            visitBlock(funcDecl->body.value().get());
        }

        leaveScope();
    }

    void visitStructDeclaration(StructDeclaration* structDecl) {
        // Handle struct declarations if needed
    }

    void visitBlock(Block* block) {
        enterScope();

        for (auto& item : block->items) {
            if (auto stmtItem = dynamic_cast<BlockItemStatement*>(item.get())) {
                visitStatement(stmtItem->statement.get());
            } else if (auto declItem = dynamic_cast<BlockItemDeclaration*>(item.get())) {
                visitDeclaration(declItem->declaration.get());
            }
        }

        leaveScope();
    }

    void visitStatement(Statement* stmt) {
        if (auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt)) {
            visitExpression(exprStmt->expression.get());
        } else if (auto returnStmt = dynamic_cast<ReturnStatement*>(stmt)) {
            if (returnStmt->expression.has_value()) {
                visitExpression(returnStmt->expression.value().get());
            }
        } else if (auto ifStmt = dynamic_cast<IfStatement*>(stmt)) {
            visitExpression(ifStmt->condition.get());
            visitStatement(ifStmt->thenBranch.get());
            if (ifStmt->elseBranch.has_value()) {
                visitStatement(ifStmt->elseBranch.value().get());
            }
        } else if (auto compoundStmt = dynamic_cast<CompoundStatement*>(stmt)) {
            visitBlock(compoundStmt->block.get());
        } else if (auto whileStmt = dynamic_cast<WhileStatement*>(stmt)) {
            visitExpression(whileStmt->condition.get());
            visitStatement(whileStmt->body.get());
        }
        // Handle other statement types as needed
    }

    void visitExpression(Exp* expr) {
        if (auto varExpr = dynamic_cast<Var*>(expr)) {
            Symbol* symbol = currentScope->lookup(varExpr->name);
            if (!symbol) {
                reportError("Use of undeclared variable '" + varExpr->name + "'");
            }
        } else if (auto binOpExpr = dynamic_cast<BinOp*>(expr)) {
            visitExpression(binOpExpr->lhs.get());
            visitExpression(binOpExpr->rhs.get());
        } else if (auto unOpExpr = dynamic_cast<UnOp*>(expr)) {
            visitExpression(unOpExpr->operand.get());
        } else if (auto funcCallExpr = dynamic_cast<FunctionCall*>(expr)) {
            Symbol* symbol = currentScope->lookup(funcCallExpr->name);
            if (!symbol) {
                reportError("Call to undeclared function '" + funcCallExpr->name + "'");
            } else if (symbol->kind != SymbolKind::Function) {
                reportError("'" + funcCallExpr->name + "' is not a function");
            }

            for (auto& arg : funcCallExpr->arguments) {
                visitExpression(arg.get());
            }
        } else if (auto assignmentExpr = dynamic_cast<Assignment*>(expr)) {
            visitExpression(assignmentExpr->lhs.get());
            visitExpression(assignmentExpr->rhs.get());
        }
        // Handle other expression types as needed
    }

    void visitInitializer(Initializer* init) {
        if (auto singleInit = dynamic_cast<SingleInit*>(init)) {
            visitExpression(singleInit->expression.get());
        } else if (auto compoundInit = dynamic_cast<CompoundInit*>(init)) {
            for (auto& subInit : compoundInit->initializers) {
                visitInitializer(subInit.get());
            }
        }
    }

    void reportError(const std::string& message) {
        std::cerr << "Semantic error: " << message << std::endl;
    }
};

#endif // SEMANTICANALYZER_HPP
