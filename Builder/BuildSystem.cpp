//
// Created by Elijah Crain on 10/27/24.
//

#include <fstream>
#include <map>
#include <vector>
#include <ranges>
#include <sstream>
#include "BuildSystem.hpp"

#include <bits/ranges_algo.h>

#include "BuildTarget.cpp"

#ifdef _WIN32
#define windowsOS true
#else
#define windowsOS false
#endif


void
BuildSystem::ParseBuildFile(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "Error: Could not open build file " << filename << std::endl;
        return;
    }

    std::string line;
    std::string current_section;
    std::map<std::string, std::string> current_properties;
    std::string current_target_type;
    std::string current_target_name;

    while (std::getline(infile, line)) {
        // Remove leading and trailing whitespace
        line.erase(line.begin(), std::ranges::find_if(line.begin(), line.end(),
            [](unsigned char ch) { return !std::isspace(ch); }));
        line.erase(std::ranges::find_if(line.rbegin(), line.rend(),
            [](unsigned char ch) { return !std::isspace(ch); }).base(), line.end());

        // Skip empty lines or comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line[0] == '[' && line.back() == ']') {
            // New section detected
            // If we have a previous section, process it
            if (!current_section.empty()) {
                // Create BuildTarget from current_properties
                std::shared_ptr<BuildTarget> target;
                if (current_target_type == "Library") {
                    target = std::make_shared<Library>();
                } else if (current_target_type == "Executable") {
                    target = std::make_shared<Executable>();
                } else {
                    std::cerr << "Error: Unknown target type '" << current_target_type << "'" << std::endl;
                    continue;
                }
                target->name = current_target_name;

                // Set properties
                for (const auto& kv : current_properties) {
                    const std::string& key = kv.first;
                    const std::string& value = kv.second;
                    if (key == "sources") {
                        target->sources = split(value, ',');
                    } else if (key == "include_dirs") {
                        target->include_dirs = split(value, ',');
                    } else if (key == "libs") {
                        target->libs = split(value, ',');
                    } else if (key == "lib_dirs") {
                        target->lib_dirs = split(value, ',');
                    } else if (key == "output_dir") {
                        target->output_dir = value;
                    } else if (key == "compiler_flags") {
                        target->compiler_flags = value;
                    } else {
                        std::cerr << "Warning: Unknown key '" << key << "'" << std::endl;
                    }
                }

                targets.push_back(target);
                current_properties.clear();
            }

            // Parse new section header
            current_section = line.substr(1, line.size() - 2); // Remove [ and ]
            size_t colon_pos = current_section.find(':');
            if (colon_pos != std::string::npos) {
                current_target_type = current_section.substr(0, colon_pos);
                current_target_name = current_section.substr(colon_pos + 1);
            } else {
                std::cerr << "Error: Invalid section header '" << current_section << "'" << std::endl;
                current_target_type.clear();
                current_target_name.clear();
            }
        } else {
            // Key-value pair
            size_t equal_pos = line.find('=');
            if (equal_pos == std::string::npos) {
                std::cerr << "Error: Invalid line in build file: " << line << std::endl;
                continue;
            }
            std::string key = line.substr(0, equal_pos);
            std::string value = line.substr(equal_pos + 1);

            // Trim whitespace from key and value
            key.erase(std::ranges::find_if(key.rbegin(), key.rend(),
                [](unsigned char ch) { return !std::isspace(ch); }).base(), key.end());
            value.erase(value.begin(), std::ranges::find_if(value.begin(), value.end(),
                [](unsigned char ch) { return !std::isspace(ch); }));

            current_properties[key] = value;
        }
    }

    // Process the last section if any
    if (!current_section.empty()) {
        std::shared_ptr<BuildTarget> target;
        if (current_target_type == "Library") {
            target = std::make_shared<Library>();
        } else if (current_target_type == "Executable") {
            target = std::make_shared<Executable>();
        } else {
            std::cerr << "Error: Unknown target type '" << current_target_type << "'" << std::endl;
        }
        target->name = current_target_name;

        // Set properties
        for (const auto& kv : current_properties) {
            const std::string& key = kv.first;
            const std::string& value = kv.second;
            if (key == "sources") {
                target->sources = split(value, ',');
            } else if (key == "include_dirs") {
                target->include_dirs = split(value, ',');
            } else if (key == "libs") {
                target->libs = split(value, ',');
            } else if (key == "lib_dirs") {
                target->lib_dirs = split(value, ',');
            } else if (key == "output_dir") {
                target->output_dir = value;
            } else if (key == "compiler_flags") {
                target->compiler_flags = value;
            } else {
                std::cerr << "Warning: Unknown key '" << key << "'" << std::endl;
            }
        }

        targets.push_back(target);
    }
    std::filesystem::create_directories(buildDir);

}

void
BuildSystem::BuildAll() const {
    for (auto& target : targets) {
      target->GenerateAssembly(buildDir);
      if (windowsOS) {
          std::cout << "Skipping assembler / linker - Only supporting arm architecture currently" << std::endl;
          return;
      }
      target->Assemble(buildDir);
      target->Link(buildDir);
    }
}

void BuildSystem::GenerateMakefile(const std::string& filename) {


    std::ofstream makefile(buildDir/filename);

    // Write variables
    makefile << "CC = your_compiler\n";
    makefile << "AS = as\n";
    makefile << "LD = ld\n";
    makefile << "CFLAGS = \n";
    makefile << "LDFLAGS = -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path`\n\n";

    // Write the 'all' target
    makefile << "all:";
    for (const auto& target : targets) {
        std::string outputFile;
        if (dynamic_cast<Library*>(target.get())) {
            outputFile = target->output_dir + "/lib" + target->name + ".a";
        } else if (dynamic_cast<Executable*>(target.get())) {
            outputFile = target->output_dir + "/" + target->name;
        }
        makefile << " " << outputFile;
    }
    makefile << "\n\n";

    // For each target, generate the Makefile rules
    for (const auto& target : targets) {
        if (auto* lib = dynamic_cast<Library*>(target.get())) {
            GenerateMakefileForLibrary(makefile, lib);
        } else if (auto* exe = dynamic_cast<Executable*>(target.get())) {
            GenerateMakefileForExecutable(makefile, exe);
        }
    }

    // Add a clean target
    makefile << "clean:\n\t";
    makefile << "rm -f *.o *.s";
    for (const auto& target : targets) {
        if (dynamic_cast<Library*>(target.get())) {
            makefile << " " << target->output_dir << "/lib" << target->name << ".a";
        } else if (dynamic_cast<Executable*>(target.get())) {
            makefile << " " << target->output_dir << "/" << target->name;
        }
    }
    makefile << "\n";

    makefile.close();
}

// Implementation for libraries
void
BuildSystem::GenerateMakefileForLibrary(std::ofstream& makefile, Library* lib) {
    std::string libFile = lib->output_dir + "/lib" + lib->name + ".a";
    std::vector<std::string> objectFiles;

    // Generate rules for each source file
    for (const auto& source : lib->sources) {
        std::string uniqueName = GetUniqueObjectName(source);
        std::string assemblyFile = uniqueName + ".s";
        std::string objectFile = uniqueName + ".o";
        objectFiles.push_back(objectFile);

        // Rule to generate assembly (.s) from source (.cej)
        makefile << assemblyFile << ": " << source << "\n\t";
        makefile << "$(CC) " << source << " -o " << assemblyFile;
        for (const auto& dir : lib->include_dirs) {
            makefile << " -I" << dir;
        }
        if (!lib->compiler_flags.empty()) {
            makefile << " " << lib->compiler_flags;
        }
        makefile << "\n\n";

        // Rule to assemble (.s to .o)
        makefile << objectFile << ": " << assemblyFile << "\n\t";
        makefile << "$(AS) -o " << objectFile << " " << assemblyFile << "\n\n";
    }

    // Rule to create the static library
    makefile << libFile << ":";
    for (const auto& obj : objectFiles) {
        makefile << " " << obj;
    }
    // Add dependencies on other libraries
    for (const auto& depLibName : lib->libs) {
        for (const auto& target : targets) {
            if (auto* depLib = dynamic_cast<Library*>(target.get())) {
                if (depLib->name == depLibName) {
                    makefile << " " << depLib->output_dir << "/lib" << depLib->name << ".a";
                }
            }
        }
    }
    makefile << "\n\tar rcs " << libFile;
    for (const auto& obj : objectFiles) {
        makefile << " " << obj;
    }
    makefile << "\n\n";
}

// Implementation for executables
void
BuildSystem::GenerateMakefileForExecutable(std::ofstream& makefile, Executable* exe) {
    std::string exeFile = exe->output_dir + "/" + exe->name;
    std::vector<std::string> objectFiles;

    // Generate rules for each source file
    for (const auto& source : exe->sources) {
        std::string uniqueName = GetUniqueObjectName(source);
        std::string assemblyFile = uniqueName + ".s";
        std::string objectFile = uniqueName + ".o";
        objectFiles.push_back(objectFile);

        // Rule to generate assembly (.s) from source (.cej)
        makefile << assemblyFile << ": " << source << "\n\t";
        makefile << "$(CC) " << source << " -o " << assemblyFile;
        for (const auto& dir : exe->include_dirs) {
            makefile << " -I" << dir;
        }
        if (!exe->compiler_flags.empty()) {
            makefile << " " << exe->compiler_flags;
        }
        makefile << "\n\n";

        // Rule to assemble (.s to .o)
        makefile << objectFile << ": " << assemblyFile << "\n\t";
        makefile << "$(AS) -o " << objectFile << " " << assemblyFile << "\n\n";
    }

    // Rule to link the executable
    makefile << exeFile << ":";
    for (const auto& obj : objectFiles) {
        makefile << " " << obj;
    }
    // Add dependencies on libraries
    for (const auto& libName : exe->libs) {
        for (const auto& target : targets) {
            if (auto* lib = dynamic_cast<Library*>(target.get())) {
                if (lib->name == libName) {
                    makefile << " " << lib->output_dir << "/lib" << lib->name << ".a";
                }
            }
        }
    }
    makefile << "\n\t$(LD) -o " << exeFile;
    for (const auto& obj : objectFiles) {
        makefile << " " << obj;
    }
    // Add library directories
    for (const auto& dir : exe->lib_dirs) {
        makefile << " -L" << dir;
    }
    // Link libraries
    for (const auto& lib : exe->libs) {
        makefile << " -l" << lib;
    }
    if (!exe->compiler_flags.empty()) {
        makefile << " " << exe->compiler_flags;
    }
    makefile << " $(LDFLAGS)\n\n";
}

// Helper function to generate unique object and assembly filenames
std::string
BuildSystem::GetUniqueObjectName(const std::string& sourceFile) {
    std::string name = sourceFile;
    std::ranges::replace(name, '/', '_');
    std::ranges::replace(name, '\\', '_');
    std::ranges::replace(name, '.', '_');
    return name;
}

// Helper function to split strings based on a delimiter and trim whitespace
std::vector<std::string>
BuildSystem::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string item;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, item, delimiter)) {
        // Trim leading and trailing whitespace
        item.erase(item.begin(), std::ranges::find_if(item,
                                                      [](unsigned char ch) { return !std::isspace(ch); }));
        item.erase(std::ranges::find_if(item.rbegin(), item.rend(),
            [](const unsigned char ch) { return !std::isspace(ch); }).base(), item.end());
        tokens.push_back(item);
    }
    return tokens;
}
