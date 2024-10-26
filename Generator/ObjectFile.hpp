//
// Created by Elijah Crain on 10/25/24.
//

#ifndef OBJECTFILE_HPP
#define OBJECTFILE_HPP

#include <string>
#include <unordered_map>
#include <vector>

class ObjectFile {
  public:
  std::string code; // The assembly code
  std::unordered_map<std::string, int> symbolTable; // Symbols defined in this object file
  std::unordered_map<std::string, std::vector<int>> unresolvedSymbols; // Symbols used but not defined in this object file
};

#endif //OBJECTFILE_HPP
