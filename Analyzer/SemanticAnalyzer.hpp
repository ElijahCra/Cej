// SemanticAnalyzer.hpp
#ifndef SEMANTICANALYZER_HPP
#define SEMANTICANALYZER_HPP

#include <stdexcept>
#include "../Parser/ParserTypes.hpp"
#include "SymbolTable.hpp"

class SemanticAnalyzer {
public:
  SemanticAnalyzer() = default;

  void
  analyze(const Program& program) {
    symbolTable.enterScope();
    for (auto& decl : program.declarations) {
      analyzeDeclaration(*decl);
    }
    symbolTable.exitScope();
      std::cout << "\nAST Passed Analsis Step \n";
  }

private:
  SymbolTable symbolTable;

  void
  analyzeDeclaration(Declaration& decl) {
    if (const auto varDecl = dynamic_cast<VariableDeclaration*>(&decl)) {
      analyzeVariableDeclaration(*varDecl);
    } else if (const auto funcDecl = dynamic_cast<FunctionDeclaration*>(&decl)) {
      analyzeFunctionDeclaration(*funcDecl);
    } else if (const auto structDecl = dynamic_cast<StructDeclaration*>(&decl)) {
      analyzeStructDeclaration(*structDecl);
    } else {
        // Handle other declaration types
    }
  }

  void
  analyzeVariableDeclaration(const VariableDeclaration& varDecl) {
    // Check if variable is already declared in current scope
    if (!symbolTable.insert(varDecl.name, Symbol(varDecl.name, varDecl.varType->clone(), Symbol::Kind::Variable))) {
      throw std::runtime_error("Variable '" + varDecl.name + "' is already declared in this scope.");
    }
    //analyze the initializer
    if (varDecl.initializer.has_value()) {
      analyzeInitializer(*varDecl.initializer.value());
    }
  }

  void
  analyzeInitializer(Initializer& init) {
    if (const auto singleInit = dynamic_cast<SingleInit*>(&init)) {
      analyzeExpression(*singleInit->expression);
    } else if (const auto compoundInit = dynamic_cast<CompoundInit*>(&init)) {
      for (auto& subInit : compoundInit->initializers) {
        analyzeInitializer(*subInit);
      }
    }
  }

  void
  analyzeFunctionDeclaration(const FunctionDeclaration& funcDecl) {
    // Build function type
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.reserve(funcDecl.parameters.size());
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

  void
  analyzeStructDeclaration(StructDeclaration& structDecl) {
    // Store the struct type in the symbol table
    auto structType = std::make_unique<StructType>(structDecl.tag);

    if (!symbolTable.insert(structDecl.tag, Symbol(structDecl.tag, std::move(structType), Symbol::Kind::Struct))) {
      throw std::runtime_error("Struct '" + structDecl.tag + "' is already declared.");
    }
      //future add struct members
  }

  void
  analyzeBlock(const Block& block) {
    symbolTable.enterScope();
    for (auto& item : block.items) {
      analyzeBlockItem(*item);
    }
    symbolTable.exitScope();
  }

  void
  analyzeBlockItem(BlockItem& item) {
    if (const auto stmtItem = dynamic_cast<BlockItemStatement*>(&item)) {
      analyzeStatement(*stmtItem->statement);
    } else if (const auto declItem = dynamic_cast<BlockItemDeclaration*>(&item)) {
      analyzeDeclaration(*declItem->declaration);
    }
  }

  void
  analyzeStatement(Statement& stmt) {
    if (const auto exprStmt = dynamic_cast<ExpressionStatement*>(&stmt)) {
      analyzeExpression(*exprStmt->expression);
    } else if (const auto returnStmt = dynamic_cast<ReturnStatement*>(&stmt)) {
      if (returnStmt->expression.has_value()) {
        analyzeExpression(*returnStmt->expression.value());
      }
    } else if (const auto compoundStmt = dynamic_cast<CompoundStatement*>(&stmt)) {
      analyzeBlock(*compoundStmt->block);
    } else if (const auto ifStmt = dynamic_cast<IfStatement*>(&stmt)) {
      analyzeExpression(*ifStmt->condition);
      analyzeStatement(*ifStmt->thenBranch);
      if (ifStmt->elseBranch.has_value()) {
        analyzeStatement(*ifStmt->elseBranch.value());
      }
    } else if (const auto whileStmt = dynamic_cast<WhileStatement*>(&stmt)) {
      analyzeExpression(*whileStmt->condition);
      analyzeStatement(*whileStmt->body);
    } else if (const auto forStmt = dynamic_cast<ForStatement*>(&stmt)) {
      symbolTable.enterScope();
      if (const auto initDecl = dynamic_cast<InitDecl*>(forStmt->init.get())) {
        analyzeVariableDeclaration(*initDecl->declaration);
      } else if (const auto initExp = dynamic_cast<InitExp*>(forStmt->init.get())) {
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
  }

  void
  analyzeExpression(Exp& exp) {
    if (const auto varExp = dynamic_cast<Var*>(&exp)) {
      // Check if variable is declared
      auto symbol = symbolTable.lookup(varExp->name);
      if (!symbol) {
          throw std::runtime_error("Use of undeclared variable '" + varExp->name + "'.");
      }
    } else if (const auto binOpExp = dynamic_cast<BinOp*>(&exp)) {
      analyzeExpression(*binOpExp->lhs);
      analyzeExpression(*binOpExp->rhs);
    } else if (const auto unOpExp = dynamic_cast<UnOp*>(&exp)) {
      analyzeExpression(*unOpExp->operand);
    } else if (const auto assignExp = dynamic_cast<Assignment*>(&exp)) {
      analyzeExpression(*assignExp->lhs);
      analyzeExpression(*assignExp->rhs);
    } else if (const auto funcCallExp = dynamic_cast<FunctionCall*>(&exp)) {
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
