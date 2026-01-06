#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <llvm/Support/FileSystem.h>

#include <filesystem>
#include <iostream>
#include <string>
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

static void cleanup_orphaned_files(
    const std::vector<ComponentInfo>& components,
    const std::filesystem::path& generated_dir,
    const std::filesystem::path& components_path)
{
	//// Build sets of expected filenames
	//std::unordered_set<std::string> expected_rttr_files;
	//std::unordered_set<std::string> expected_component_files;

	//for (const auto& comp : components) {
	//	expected_rttr_files.insert(comp.generated_code_path.filename().string());
	//	expected_component_files.insert(comp.generated_component_file.filename().string());
	//}

	//// Always keep component_rttr.cpp
	//expected_rttr_files.insert("component_rttr.cpp");

	//// Clean up generated RTTR files
	//for (const auto& entry : std::filesystem::directory_iterator(generated_dir)) {
	//	if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
	//		const std::string filename = entry.path().filename().string();
	//		if (expected_rttr_files.find(filename) == expected_rttr_files.end()) {
	//			log("Removing orphaned RTTR file: {}", entry.path().string());
	//			std::filesystem::remove(entry.path());
	//		}
	//	}
	//}

	//// Clean up generated component files
	//if (std::filesystem::exists(components_path)) {
	//	for (const auto& entry : std::filesystem::directory_iterator(components_path)) {
	//		if (entry.is_regular_file() && entry.path().extension() == ".component") {
	//			const std::string filename = entry.path().filename().string();
	//			if (expected_component_files.find(filename) == expected_component_files.end()) {
	//				log("Removing orphaned component file: {}", entry.path().string());
	//				std::filesystem::remove(entry.path());
	//			}
	//		}
	//	}
	//}
}

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <compile_commands_dir>\n";
        return 1;
    }

    const std::string compile_commands_dir = argv[1];
    std::string error_msg;
    const auto compile_db = CompilationDatabase::loadFromDirectory(compile_commands_dir, error_msg);
    if (!compile_db) {
		log("Failed to load compile commands at dir {}. Error: {}", compile_commands_dir, error_msg);
        return 1;
    }

	// NOTE: assumes a folder structure!
    const std::filesystem::path engine_source_game = 
        std::filesystem::path(compile_commands_dir).parent_path() / "source" / "game";
    
    if (!std::filesystem::exists(engine_source_game)) {
        log("Directory not found: {}", engine_source_game.string());
        return 1;
    }

    const std::vector<std::string> game_cppm_files = filter_cppm_files(engine_source_game);

    if (game_cppm_files.empty()) {
        log("No .cppm files found in: {} ", engine_source_game.string());
        return 1;
    }

	// exported .component files
	const std::filesystem::path components_path =
        std::filesystem::absolute(std::filesystem::path(compile_commands_dir).parent_path().parent_path() / "shared_resources" / "components");

    std::filesystem::create_directories(components_path);

	// generated code files
	const std::filesystem::path generated_dir =
        std::filesystem::absolute(std::filesystem::path(compile_commands_dir).parent_path() / "source" / "game" / "generated" / "rttr_register");
    std::filesystem::create_directories(generated_dir);

    ClangTool Tool(*compile_db, game_cppm_files);

    ComponentMatchCallback Callback;
	Callback.code_path = generated_dir;
	Callback.component_path = components_path;

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
    generate_rttr_registration(Callback.components);

	// Cleanup orphaned files
	cleanup_orphaned_files(Callback.components, generated_dir, components_path);
}
