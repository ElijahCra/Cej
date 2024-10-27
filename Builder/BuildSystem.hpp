//
// Created by Elijah Crain on 10/27/24.
//

#ifndef BUILDSYSTEM_HPP
#define BUILDSYSTEM_HPP

#include "BuildTarget.hpp"
#include "Library.cpp"
#include "Executable.cpp"
#include <vector>

class BuildSystem {
  public:
    std::vector<std::shared_ptr<BuildTarget>> targets;
    void GenerateMakefile(const std::string& filename);
    std::vector<std::string> split(const std::string &s, char delimiter);
    void ParseBuildFile(const std::string &filename);
    void BuildAll();

  private:
    void GenerateMakefileForLibrary(std::ofstream& makefile, Library* lib);
    void GenerateMakefileForExecutable(std::ofstream& makefile, Executable* exe);
    std::string GetUniqueObjectName(const std::string& sourceFile);
};


#endif //BUILDSYSTEM_HPP
