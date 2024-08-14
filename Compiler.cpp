//
// Created by Elijah on 8/13/2024.
//
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include "Lexer.cpp"
#include "Parser.cpp"
#include "Generator.cpp"

class Compiler {
public:
    void compile(const std::string& inputFile) {
        std::string content = readFile(inputFile);
        Lexer lexer;
        Parser parser;
        ARMCodeGenerator codeGen;

        auto tokens = lexer.tokenize(content);
        auto ast = parser.parse(tokens);
        std::string armCode = codeGen.generateCode(ast.get());

        std::string asmFile = inputFile + ".s";
        writeFile(asmFile, armCode);

        assembleAndLink(asmFile);
    }

private:
    std::string readFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            throw std::runtime_error("Unable to open file: " + filename);
        }
        return std::string((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
    }

    void writeFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        if (!file) {
            throw std::runtime_error("Unable to write file: " + filename);
        }
        file << content;
    }

    void assembleAndLink(const std::string& asmFile) {
        std::string objFile = asmFile + ".o";
        std::string exeFile = asmFile + ".exe";

        // Assemble
        std::string asCmd = "as -o " + objFile + " " + asmFile;
        if (system(asCmd.c_str()) != 0) {
            throw std::runtime_error("Assembly failed");
        }

        // Link
        std::string ldCmd = "ld -o " + exeFile + " " + objFile;
        if (system(ldCmd.c_str()) != 0) {
            throw std::runtime_error("Linking failed");
        }

        // Clean up
        std::remove(asmFile.c_str());
        std::remove(objFile.c_str());

        std::cout << "Executable created: " << exeFile << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file.cej>" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    if (inputFile.substr(inputFile.length() - 4) != ".cej") {
        std::cerr << "Input file must have .cej extension" << std::endl;
        return 1;
    }

    try {
        Compiler compiler;
        compiler.compile(inputFile);
    } catch (const std::exception& e) {
        std::cerr << "Compilation error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}