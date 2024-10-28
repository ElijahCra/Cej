//
// Created by Elijah Crain on 10/27/24.
//

#include "BuildTarget.hpp"
#include "../Lexer/Lexer.cpp"
#include "../Parser/Parser.cpp"
#include "../Generator/Generator.cpp"

void BuildTarget::GenerateAssembly(const std::filesystem::path& buildDir) {
  for (const auto& source : sources) {
    Lexer lexer(source);
    Parser parser(lexer);
    auto tree = parser.Parse();
    std::string outData = Generator::GenerateAssembly(tree);

    // Generate output filename
    size_t front = source.find_first_of('/')== std::string::npos ? 0 : source.find_last_of('/') + 1;
    std::string assemblyFile = source.substr(front, source.find_last_of('.')-front) + ".s";
    std::ofstream outFile(buildDir / assemblyFile);
    outFile.write(outData.data(), outData.size());
    outFile.close();
  }
}

void BuildTarget::Assemble(const std::filesystem::path& buildDir) {
  for (const auto& source : sources) {
    std::filesystem::path assemblyFile = buildDir / std::string(source.substr(0, source.find_last_of('.')) + ".s");
    std::filesystem::path objectFile = buildDir / std::string(source.substr(0, source.find_last_of('.')) + ".o");

    std::string command = "as -o " + objectFile.string() + " " + assemblyFile.string();
    system(command.c_str());
  }
}

