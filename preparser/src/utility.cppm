module;

#include <string>
#include <vector>
#include <filesystem>
#include <optional>
#include <fstream>
#include <algorithm>
#include <cctype>

#include <clang/ASTMatchers/ASTMatchers.h>

export module preparser.utility;

export std::vector<std::string> filter_cppm_files(const std::filesystem::path& dir) {
    std::vector<std::string> files;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
        if (entry.path().extension() == ".cppm") {
            files.push_back(entry.path().string());
        }
    }
    return files;
}

export bool has_annotation(const clang::Decl* D, const std::string& annotation) {
    for (const auto* Attr : D->attrs()) {
        if (const auto* Ann = dyn_cast<clang::AnnotateAttr>(Attr)) {
            if (Ann->getAnnotation() == annotation) {
                return true;
            }
        }
    }
    return false;
}

export bool has_any_annotation(const clang::Decl* D) {
    for (const auto* Attr : D->attrs()) {
        if (dyn_cast<clang::AnnotateAttr>(Attr)) {
            return true;
        }
    }
    return false;
}

export std::optional<std::string> get_annotation_value(const clang::Decl* D) {
    for (const auto* Attr : D->attrs()) {
        if (const auto* Ann = dyn_cast<clang::AnnotateAttr>(Attr)) {
            return Ann->getAnnotation().str();
        }
    }
    return {};
}

export std::optional<std::string> extract_module_name(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return {};
    }

    std::string line;
    while (std::getline(file, line)) {
        // Look for "export module <name>;"
        size_t export_pos = line.find("export module");
        if (export_pos != std::string::npos) {
            size_t module_start = export_pos + 13; // length of "export module"
            size_t semicolon = line.find(';', module_start);
            if (semicolon != std::string::npos) {
                std::string module = line.substr(module_start, semicolon - module_start);
                module.erase(0, module.find_first_not_of(" \t"));
                module.erase(module.find_last_not_of(" \t") + 1);
                return module;
            }
        }
    }

	return {};
}

export std::optional<std::string> extract_enable_if_method(std::string value) {
    auto trim = [](std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    };

    trim(value);
    if (value.empty()) {
        return {};
    }

    const char first = value.front();
    const char last = value.back();
    if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
        value = value.substr(1, value.size() - 2);
    }

    trim(value);
    if (value.empty()) {
        return {};
    }

    if (value.ends_with("()")) {
        value.erase(value.size() - 2);
        trim(value);
    }

    if (value.empty()) {
        return {};
    }

    return value;
}
