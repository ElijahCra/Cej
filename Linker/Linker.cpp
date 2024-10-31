//
// Created by Elijah Crain on 10/25/24.
//

#ifndef LINKER_CPP
#define LINKER_CPP

// Linker.cpp
#ifndef LINKER_CPP
#define LINKER_CPP

#include <string>
#include <unordered_map>
#include <vector>
#include "ObjectFile.h"

class Linker {
  public:
  std::string Link(const std::vector<ObjectFile>& objectFiles) {
    // Global symbol table
    std::unordered_map<std::string, int> globalSymbolTable;

    // Collect all symbols defined in all object files
    int currentAddress = 0;
    std::vector<std::string> codeSegments;
    std::vector<int> segmentAddresses;

    for (const auto& objFile : objectFiles) {
      // Record the starting address of this code segment
      segmentAddresses.push_back(currentAddress);

      // For each symbol defined in this object file, assign an address
      for (const auto& symbol : objFile.symbolTable) {
        std::string symbolName = symbol.first;
        int symbolOffset = symbol.second;
        globalSymbolTable[symbolName] = currentAddress + symbolOffset;
      }

      // Update currentAddress by the length of this object's code (in lines)
      int codeLength = std::count(objFile.code.begin(), objFile.code.end(), '\n');
      codeSegments.push_back(objFile.code);
      currentAddress += codeLength; // Simplified address calculation
    }

    // Now, resolve unresolved symbols
    // Since we're dealing with assembly code and labels, we'll combine the code segments and let the assembler handle the labels

    // Combine code segments
    std::string finalCode;
    for (const auto& code : codeSegments) {
      finalCode += code;
    }

    return finalCode;
  }
};

#endif // LINKER_CPP


#endif //LINKER_CPP