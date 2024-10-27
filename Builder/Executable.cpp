//
// Created by Elijah Crain on 10/27/24.
//

#include "BuildTarget.hpp"

class Executable : public BuildTarget {
  public:
  void Link() override;
};

void Executable::Link() {
  std::string executableFile = output_dir + "/" + name;
  std::string command = "ld -o " + executableFile;

  // Add object files
  for (const auto& source : sources) {
    std::string objectFile = source.substr(0, source.find_last_of('.')) + ".o";
    command += " " + objectFile;
  }

  // Add library directories
  for (const auto& dir : lib_dirs) {
    command += " -L" + dir;
  }

  // Add libraries
  for (const auto& lib : libs) {
    command += " -l" + lib;
  }

  // Add system libraries (from your template command)
  command += " -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path`";

  system(command.c_str());
}
