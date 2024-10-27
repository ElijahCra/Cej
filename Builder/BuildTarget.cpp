//
// Created by Elijah Crain on 10/27/24.
//

#include "BuildTarget.hpp"
#include "../Lexer/Lexer.cpp"
#include "../Parser/Parser.cpp"
#include "../Generator/Generator.cpp"

void BuildTarget::GenerateAssembly() {
  for (const auto& source : sources) {
    Lexer lexer(source);
    Parser parser(lexer);
    auto tree = parser.Parse();
    std::string outData = Generator::GenerateAssembly(tree);

    // Generate output filename
    std::string assemblyFile = source.substr(0, source.find_last_of('.')) + ".s";
    std::ofstream outFile(assemblyFile);
    outFile.write(outData.data(), outData.size());
    outFile.close();
  }
}

void BuildTarget::Assemble() {
  for (const auto& source : sources) {
    std::string assemblyFile = source.substr(0, source.find_last_of('.')) + ".s";
    std::string objectFile = source.substr(0, source.find_last_of('.')) + ".o";

    std::string command = "as -o " + objectFile + " " + assemblyFile;
    system(command.c_str());
  }
}

