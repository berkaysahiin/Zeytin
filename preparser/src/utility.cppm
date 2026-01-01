module;

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

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
