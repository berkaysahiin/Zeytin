#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/ArgumentsAdjusters.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <llvm/Support/FileSystem.h>

#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

import preparser.logger;
import preparser.utility;
import preparser.types;
import preparser.matchers.component;
import preparser.jsonexport;
import preparser.rttr_generator;

struct Options {
    std::filesystem::path compile_commands_dir;
    std::filesystem::path game_source_dir;
    std::filesystem::path generated_code_file;
    std::filesystem::path component_metadata_dir;
    std::filesystem::path manifest_file;
};

static bool parse_options(const int argc, const char** argv, Options& options) {
    if (argc != 11) {
        return false;
    }

    for (int i = 1; i < argc; i += 2) {
        const std::string_view option = argv[i];
        const std::filesystem::path value = argv[i + 1];

        if (option == "--compile-commands") {
            options.compile_commands_dir = value;
        } else if (option == "--game-source") {
            options.game_source_dir = value;
        } else if (option == "--generated-code") {
            options.generated_code_file = value;
        } else if (option == "--component-metadata") {
            options.component_metadata_dir = value;
        } else if (option == "--manifest") {
            options.manifest_file = value;
        } else {
            return false;
        }
    }

    return !options.compile_commands_dir.empty()
        && !options.game_source_dir.empty()
        && !options.generated_code_file.empty()
        && !options.component_metadata_dir.empty()
        && !options.manifest_file.empty();
}

static void write_manifest(
    const std::vector<ComponentInfo>& components,
    const std::filesystem::path& manifest_file)
{
    std::vector<std::filesystem::path> metadata_files;
    metadata_files.reserve(components.size());
    for (const auto& component : components) {
        metadata_files.push_back(component.generated_component_file);
    }
    std::ranges::sort(metadata_files);

    std::ofstream manifest(manifest_file);
    if (!manifest.is_open()) {
        throw std::runtime_error(std::format(
            "Failed to open manifest file: {}", manifest_file.string()));
    }

    for (const auto& metadata_file : metadata_files) {
        manifest << metadata_file.string() << '\n';
    }
}

static void cleanup_orphaned_files(
    const std::vector<ComponentInfo>& components,
    const std::filesystem::path& generated_code_file,
    const std::filesystem::path& components_path)
{
	std::unordered_set<std::string> expected_component_files;

	for (const auto& comp : components) {
		expected_component_files.insert(comp.generated_component_file.filename().string());
	}

	// Remove per-component registration files left by the previous generator.
	for (const auto& entry : std::filesystem::directory_iterator(generated_code_file.parent_path())) {
		if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
			if (entry.path() != generated_code_file) {
				log("Removing orphaned RTTR file: {}", entry.path().string());
				std::filesystem::remove(entry.path());
			}
		}
	}

	// Clean up generated component files
	if (std::filesystem::exists(components_path)) {
		for (const auto& entry : std::filesystem::directory_iterator(components_path)) {
			if (entry.is_regular_file() && entry.path().extension() == ".component") {
				const std::string filename = entry.path().filename().string();
				if (expected_component_files.find(filename) == expected_component_files.end()) {
					log("Removing orphaned component file: {}", entry.path().string());
					std::filesystem::remove(entry.path());
				}
			}
		}
	}
}

int main(int argc, const char** argv) {
    Options options;
    if (!parse_options(argc, argv, options)) {
        std::cerr
            << "Usage: " << argv[0]
            << " --compile-commands <dir>"
            << " --game-source <dir>"
            << " --generated-code <file>"
            << " --component-metadata <dir>"
            << " --manifest <file>\n";
        return 1;
    }

    std::string error_msg;
    const auto compile_db = CompilationDatabase::loadFromDirectory(
        options.compile_commands_dir.string(), error_msg);
    if (!compile_db) {
		log("Failed to load compile commands at dir {}. Error: {}",
            options.compile_commands_dir.string(), error_msg);
        return 1;
    }

    if (!std::filesystem::is_directory(options.game_source_dir)) {
		log("Directory not found: {}", options.game_source_dir.string());
        return 1;
    }

    const std::vector<std::string> game_cppm_files = filter_cppm_files(options.game_source_dir);

    if (game_cppm_files.empty()) {
		log("No .cppm files found in: {}", options.game_source_dir.string());
        return 1;
    }

	log("Compile commands dir: {}", options.compile_commands_dir.string());
	log("Game source dir: {}", options.game_source_dir.string());
	log("Generated code file: {}", options.generated_code_file.string());
	log("Component metadata dir: {}", options.component_metadata_dir.string());
	log("Manifest file: {}", options.manifest_file.string());

    std::filesystem::create_directories(options.generated_code_file.parent_path());
    std::filesystem::create_directories(options.component_metadata_dir);
    std::filesystem::create_directories(options.manifest_file.parent_path());

    ClangTool Tool(*compile_db, game_cppm_files);
    Tool.appendArgumentsAdjuster(getInsertArgumentAdjuster(
        {std::string("-resource-dir=") + ZEYTIN_CLANG_RESOURCE_DIR},
        ArgumentInsertPosition::BEGIN));

    ComponentMatchCallback Callback;
	Callback.code_path = options.generated_code_file.parent_path();
	Callback.component_path = options.component_metadata_dir;

    MatchFinder Finder;
    	Finder.addMatcher(
			cxxRecordDecl(
    isDefinition(),
    isDerivedFrom(hasName("Component"))
	).bind("component"), &Callback);

    const bool rv = Tool.run(newFrontendActionFactory(&Finder).get());

	if(rv != 0) {
		log("Error, will not export or generate code");
		return -1;
	}

	export_components(Callback.components);
    generate_rttr_registration(Callback.components, options.generated_code_file);

	cleanup_orphaned_files(
        Callback.components,
        options.generated_code_file,
        options.component_metadata_dir);
    write_manifest(Callback.components, options.manifest_file);
}
