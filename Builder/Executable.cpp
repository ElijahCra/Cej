//
// Created by Elijah Crain on 10/27/24.
//

#include "BuildTarget.hpp"

class Executable : public BuildTarget {
  public:
  void Link(const std::filesystem::path& buildDir) override;
};

void Executable::Link(const std::filesystem::path& buildDir) {
  const std::filesystem::path execDir = buildDir / output_dir;
  if (!std::filesystem::exists(execDir)) {
    std::filesystem::create_directory(execDir);
  }
  const std::filesystem::path executableFile = execDir / name;
  std::string command = "ld -o " + executableFile.string();

  // Add object files
  for (const auto& source : sources) {
    std::string objectFile = source.substr(0, source.find_last_of('.')) + ".o";
    command += " " + (buildDir / (objectFile)).string();
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
