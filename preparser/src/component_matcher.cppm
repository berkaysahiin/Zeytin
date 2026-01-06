module;

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <filesystem>

export module preparser.matchers.component;
import preparser.types;

export class ComponentMatchCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult& Result) override;
	std::vector<ComponentInfo> components;
	std::filesystem::path code_path;
	std::filesystem::path component_path;
};


