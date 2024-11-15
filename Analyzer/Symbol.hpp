//
// Created by Elijah Crain on 11/13/24.
//

#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <string>
#include <memory>
#include "../Parser/ParserTypes.hpp"

enum class SymbolKind {
  Variable,
  Function
};

struct Symbol {
  std::string name;
  Type type;
  SymbolKind kind;

  Symbol(std::string n, Type t, SymbolKind k)
      : name(std::move(n)), type(t), kind(k) {}
};


#endif //SYMBOL_HPP
