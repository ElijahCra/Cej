//
// Created by Elijah Crain on 10/27/24.
//

#include "BuildTarget.hpp"

class Library : public BuildTarget {
  public:
  void Link(const std::filesystem::path& buildDir) override;
};

void Library::Link(const std::filesystem::path& buildDir) {
  // Create the library file
  const std::filesystem::path libDir = buildDir / output_dir;
  std::string libFile = (libDir / (name + ".a")).string();
  if (!std::filesystem::exists(libDir)) {
    std::filesystem::create_directory(libDir);
  }
  std::string command = "ar rcs " + libFile;

  for (const auto& source : sources) {
    std::string objectFile = source.substr(0, source.find_last_of('.')) + ".o";
    command += " " + objectFile;
  }
  system(command.c_str());
}

