//
// Created by Elijah Crain on 10/25/24.
//

#ifndef BUILDCONFIG_HPP
#define BUILDCONFIG_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include "../Parser/Parser.cpp"
#include "../Generator/Generator.cpp"
#include "../Lexer/Lexer.cpp"

namespace fs = std::filesystem;

struct BuildConfig {
    std::string type; // "executable" or "library"
    std::string name;
    std::vector<std::string> files;
    std::map<std::string, std::string> options; // e.g., optimization level, include paths
};

BuildConfig parseBuildFile(const std::string &filepath) {
    BuildConfig config;
    std::ifstream buildFile(filepath);
    if (!buildFile.is_open()) {
        throw std::runtime_error("Failed to open build file: " + filepath);
    }

    std::string line;
    while (std::getline(buildFile, line)) {
        if (line.find("type:") == 0) {
            config.type = line.substr(5);
        } else if (line.find("name:") == 0) {
            config.name = line.substr(5);
        } else if (line.find("file:") == 0) {
            config.files.push_back(line.substr(5));
        } else if (line.find("option:") == 0) {
            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = line.substr(7, delimiterPos - 7);
                std::string value = line.substr(delimiterPos + 1);
                config.options[key] = value;
            }
        }
    }

    return config;
}

void compileFiles(const BuildConfig &config) {
    std::vector<std::string> objectFiles;

    for (const auto &file : config.files) {
        std::cout << "Compiling: " << file << "...\n";
        // Lexer, Parser, and Generator logic would go here
        Lexer lexer(file);
        Parser parser(lexer);
        std::string out = Generator::GenerateAssembly(parser.Parse());

        std::string objectFile = file + ".o";

        objectFiles.push_back(objectFile);
    }

    // Link object files
    std::string outputFile = config.name;
    if (config.type == "executable") {
        outputFile += ".out";
    } else if (config.type == "library") {
        outputFile = "lib" + outputFile + ".so";
    }

    std::cout << "Linking files to create: " << outputFile << "...\n";
    // Linking logic would go here (e.g., using system linker)
}




#endif //BUILDCONFIG_HPP
