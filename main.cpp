#include <format>
#include <iostream>

#include "Builder/BuildConfig.hpp"
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

    // You can now link 'output.o' with other object files to create an executable
    return 0;
  }
  if (1 == argc && argv[0] == std::string("--help")) {
    std::cout << "Usage: compiler [-f <file>] [-b <build file>] [options]\n";
    return 0;
  }
  if (1 == argc && argv[0] == std::string("--version")) {
    std::cout << "version 0.1\n";
    return 0;
  }
  if (1 == argc){
    std::cerr << "Usage: compiler [-f <file>] [-b <build file>] [options]\n";
    return 1;
  }
  if (2 >= argc) {
    std::string mode;
    std::string filepath;
    BuildConfig config;

    for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg == "-f" && i + 1 < argc) {
        mode = "file";
        filepath = argv[++i];
      } else if (arg == "-b" && i + 1 < argc) {
        mode = "build";
        filepath = argv[++i];
      } else if (arg == "-o" && i + 1 < argc) {
        config.options["optimization"] = argv[++i];
      } else if (arg == "-I" && i + 1 < argc) {
        config.options["include_path"] = argv[++i];
      } else if (arg == "-v") {
        config.options["verbose"] = "true";
      }
    }

    try {
      if (mode == "file") {
        config.type = "executable";
        config.name = "output";
        config.files.push_back(filepath);
      } else if (mode == "build") {
        config = parseBuildFile(filepath);
      } else {
        throw std::runtime_error("Invalid mode specified.");
      }
      compileFiles(config);
    } catch (const std::exception &e) {
      std::cerr << "Error: " << e.what() << "\n";
      return 1;
    }
    return 0;
  }
}