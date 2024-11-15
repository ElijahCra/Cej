//
// Created by Elijah Crain on 11/13/24.
//

#ifndef SYMBOLTABLE_HPP
#define SYMBOLTABLE_HPP

#include <unordered_map>
#include <string>
#include <memory>
#include "Symbol.hpp"

class SymbolTable {

  std::vector<std::unordered_map<std::string, Symbol>> scopes;

public:
  SymbolTable() {
    enterScope(); // Start with a global scope
  }

  void enterScope() {
    scopes.emplace_back();
  }

  void exitScope() {
    scopes.pop_back();
  }

  bool insert(const std::string& name, Symbol entry) {
    auto& currentScope = scopes.back();
    auto result = currentScope.emplace(name, std::move(entry));
    return result.second; // returns true if insertion took place
  }

  Symbol* lookup(const std::string& name) {
    for (auto scopeIter = scopes.rbegin(); scopeIter != scopes.rend(); ++scopeIter) {
      auto& scope = *scopeIter;
      auto it = scope.find(name);
      if (it != scope.end()) {
        return &(it->second);
      }
    }
    return nullptr;
  }
};

#endif //SYMBOLTABLE_HPP
