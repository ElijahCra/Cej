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
  private:
  std::unordered_map<std::string, Symbol> table;
  SymbolTable* parent;

  public:
  SymbolTable(SymbolTable* p = nullptr) : parent(p) {}

  bool declare(const std::string& name, const Symbol& symbol) {
    if (table.find(name) != table.end()) {
      // Symbol already declared in this scope
      return false;
    }
    table.insert(std::make_pair(name,symbol));
    return true;
  }

  Symbol* lookup(const std::string& name) {
    auto it = table.find(name);
    if (it != table.end()) {
      return &it->second;
    } else if (parent) {
      return parent->lookup(name);
    }
    return nullptr;
  }

  SymbolTable* getParent() {
    return parent;
  }

  ~SymbolTable() {
    delete parent;
  }
};

#endif //SYMBOLTABLE_HPP
