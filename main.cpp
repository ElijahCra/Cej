#include <format>
#include <iostream>

#include "Builder/BuildSystem.cpp"
#include "Generator/Generator.cpp"
#include "Parser/Parser.cpp"

int main(int argc, char *argv[]) {
  if (1 == argc && **argv == '/') {
    Lexer lexer("../example.cej");
    Parser parser(lexer);
    auto tree = parser.Parse();
    std::string outData = Generator::GenerateAssembly(tree);

    std::ofstream outFile("output.s");
    outFile.write(outData.data(), outData.size());
    outFile.close();

    return 0;
  }
  BuildSystem buildSystem;
  buildSystem.ParseBuildFile("cej.build");
  buildSystem.BuildAll();
  buildSystem.GenerateMakefile("Makefile");
  return 0;
}