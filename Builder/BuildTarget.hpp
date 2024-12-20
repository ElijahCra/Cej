//
// Created by Elijah Crain on 10/27/24.
//

#ifndef BUILDTARGET_HPP
#define BUILDTARGET_HPP

#include <vector>
#include <filesystem>

class BuildTarget {
  public:
    virtual ~BuildTarget() = default;
    std::string name;
  std::vector<std::string> sources;
  std::vector<std::string> include_dirs;
  std::vector<std::string> libs;
  std::vector<std::string> lib_dirs;
  std::string output_dir;
  std::string compiler_flags;

  virtual void GenerateAssembly(const std::filesystem::path& buildDir);
  virtual void Assemble(const std::filesystem::path& buildDir);
  virtual void Link(const std::filesystem::path& buildDir) = 0;
};


#endif //BUILDTARGET_HPP
