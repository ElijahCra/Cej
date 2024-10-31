#include <iostream>

#include "Builder/BuildSystem.cpp"
#include "Generator/Generator.cpp"
#include "Parser/Parser.cpp"



int main(int argc, char *argv[]) {
  if (1 == argc && **argv == '/') {
    Lexer lexer("../example.cej");
    Parser parser(lexer);
    auto tree = parser.parseUnit();
    std::string outData = Generator::GenerateAssembly(tree);

    std::ofstream outFile("output.s");
    outFile.write(outData.data(), outData.size());
    outFile.close();

    return 0;
  }
  if (2 == argc) {
    std::cerr << "Usage: CejCompiler -b \"buildFilePath\"";
    return 1;
  }
  if (3 == argc && std::string_view(argv[1]) == "-b") {
    BuildSystem buildSystem;
    buildSystem.ParseBuildFile(std::string(argv[2]));
    buildSystem.BuildAll();
    buildSystem.GenerateMakefile("Makefile");
    return 0;
  }

}