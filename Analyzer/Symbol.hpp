//
// Created by Elijah Crain on 11/13/24.
//

#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <string>
#include <memory>
#include "../Parser/ParserTypes.hpp"



struct Symbol {
  enum class Kind {
    Variable,
    Function,
    Struct
  };

  std::string name;
  std::unique_ptr<Type> type;
  Kind kind;

  Symbol(std::string n, std::unique_ptr<Type> t, const Kind k)
        : name(std::move(n)), type(std::move(t)), kind(k) {}
};


#endif //SYMBOL_HPP
