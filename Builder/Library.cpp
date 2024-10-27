//
// Created by Elijah Crain on 10/27/24.
//

#include "BuildTarget.hpp"

class Library : public BuildTarget {
  public:
  void Link() override;
};

void Library::Link() {
  // Create the library file
  std::string libFile = output_dir + "/lib" + name + ".a";
  std::string command = "ar rcs " + libFile;

  for (const auto& source : sources) {
    std::string objectFile = source.substr(0, source.find_last_of('.')) + ".o";
    command += " " + objectFile;
  }
  system(command.c_str());
}

