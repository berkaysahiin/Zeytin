module;

#include <clang/ASTMatchers/ASTMatchFinder.h>

export module preparser.matchers.component;
import preparser.types;

export class ComponentMatchCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult& Result) override;
	std::vector<ComponentInfo> components;
};


