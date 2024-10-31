//
// Created by Elijah Crain on 10/27/24.
//

#ifndef BUILDSYSTEM_HPP
#define BUILDSYSTEM_HPP

#include "BuildTarget.hpp"
#include "Library.cpp"
#include "Executable.cpp"
#include <vector>
#include <filesystem>

class BuildSystem {
  public:
    explicit BuildSystem() {
      sourceDir = std::filesystem::current_path();
      buildDir = std::filesystem::current_path() / "Cej-Build";
      std::cout << sourceDir << std::endl;
    }
    std::vector<std::shared_ptr<BuildTarget>> targets;
    std::filesystem::path sourceDir;
    std::filesystem::path buildDir;
    void GenerateMakefile(const std::string& filename);
    static std::vector<std::string> split(const std::string &s, char delimiter);
    void ParseBuildFile(const std::string &filename);
    void BuildAll() const;

  private:
    void GenerateMakefileForLibrary(std::ofstream& makefile, Library* lib);
    void GenerateMakefileForExecutable(std::ofstream& makefile, Executable* exe);
    static std::string GetUniqueObjectName(const std::string& sourceFile);
};


#endif //BUILDSYSTEM_HPP
