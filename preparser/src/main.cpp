#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <llvm/Support/FileSystem.h>

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

import preparser.logger;
import preparser.utility;
import preparser.matchers.component;
import preparser.jsonexport;

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <compile_commands_dir>\n";
        return 1;
    }

    std::string compile_commands_dir = argv[1];
    std::string error_msg;
    auto compile_db = CompilationDatabase::loadFromDirectory(compile_commands_dir, error_msg);
    if (!compile_db) {
		log("Failed to load compile commands at dir {}. Error: {}", compile_commands_dir, error_msg);
        return 1;
    }

	// NOTE: assumes a folder structure!
    std::filesystem::path engine_source_game = 
        std::filesystem::path(compile_commands_dir).parent_path() / "source" / "game";
    
    if (!std::filesystem::exists(engine_source_game)) {
        log("Directory not found: {}", engine_source_game.string());
        return 1;
    }

    auto game_cppm_files = filter_cppm_files(engine_source_game);

    if (game_cppm_files.empty()) {
        log("No .cppm files found in: {} ", engine_source_game.string());
        return 1;
    }

	for(const auto& game_cppm_file : game_cppm_files) {
		log("Will parse: {}", game_cppm_file);
	}

    ClangTool Tool(*compile_db, game_cppm_files);

    ComponentMatchCallback Callback;

    MatchFinder Finder;
    	Finder.addMatcher(
			cxxRecordDecl(
    isDefinition(),
    isDerivedFrom(hasName("Component"))
	).bind("component"), &Callback);

    const bool rv = Tool.run(newFrontendActionFactory(&Finder).get());

	for(const auto& component: Callback.components) 
	{
		log("---------------------------");
		debug_print_component(component);
	}

	export_components(Callback.components);
}
