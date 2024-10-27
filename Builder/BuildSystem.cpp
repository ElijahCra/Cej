//
// Created by Elijah Crain on 10/27/24.
//

#include <fstream>
#include <map>
#include <vector>
#include <ranges>
#include <sstream>
#include "BuildSystem.hpp"
#include "BuildTarget.hpp"


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
        line.erase(line.begin(), std::find_if(line.begin(), line.end(),
            [](unsigned char ch) { return !std::isspace(ch); }));
        line.erase(std::find_if(line.rbegin(), line.rend(),
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
            key.erase(std::find_if(key.rbegin(), key.rend(),
                [](unsigned char ch) { return !std::isspace(ch); }).base(), key.end());
            value.erase(value.begin(), std::find_if(value.begin(), value.end(),
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
}

void
BuildSystem::BuildAll() {
    for (auto& target : targets) {
      target->GenerateAssembly();
      target->Assemble();
      target->Link();
    }
}

void
BuildSystem::GenerateMakefile(const std::string& filename) {
    std::ofstream makefile(filename);
    // Write makefile contents based on targets
    makefile << "all: ";
    for (const auto& target : targets) {
      makefile << target->output_dir << target->name << " ";
    }
    makefile << "\n\n";

    // For each target, write the rules
    for (const auto& target : targets) {
      // Generate makefile rules for each target
      // Include dependencies, commands, etc.
    }
    makefile.close();
}

// Helper function to split strings based on a delimiter and trim whitespace
std::vector<std::string>
BuildSystem::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string item;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, item, delimiter)) {
        // Trim leading and trailing whitespace
        item.erase(item.begin(), std::find_if(item.begin(), item.end(),
            [](unsigned char ch) { return !std::isspace(ch); }));
        item.erase(std::find_if(item.rbegin(), item.rend(),
            [](unsigned char ch) { return !std::isspace(ch); }).base(), item.end());
        tokens.push_back(item);
    }
    return tokens;
}
