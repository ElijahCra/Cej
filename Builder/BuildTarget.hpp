//
// Created by Elijah Crain on 10/27/24.
//

#ifndef BUILDTARGET_HPP
#define BUILDTARGET_HPP

#include <vector>

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

  virtual void GenerateAssembly();
  virtual void Assemble();
  virtual void Link() = 0;
};


#endif //BUILDTARGET_HPP
