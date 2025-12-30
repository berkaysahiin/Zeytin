#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/FrontendActions.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <set>
#include <fstream>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
namespace fs = std::filesystem;

class ClassPrinter : public MatchFinder::MatchCallback {
public:
    virtual void run(const MatchFinder::MatchResult &Result) override {
        if (const auto *classDecl = Result.Nodes.getNodeAs<CXXRecordDecl>("class")) {
            auto loc = classDecl->getLocation();
            auto &srcMgr = Result.Context->getSourceManager();
            
            std::string filename = srcMgr.getFilename(loc).str();
            
            if (filename.ends_with(".h") || filename.ends_with(".hpp")) {
                std::cout << "Found parseable class: " << classDecl->getNameAsString() << "\n";
                std::cout << "  Location: " << filename << ":" 
                          << srcMgr.getSpellingLineNumber(loc) << "\n";
            }
        }
    }
};

std::vector<std::string> findHeaders(const std::string& directory) {
    std::vector<std::string> headers;
    
    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            auto path = entry.path().string();
            if (path.ends_with(".h") || path.ends_with(".hpp")) {
                headers.push_back(path);
            }
        }
    }
    
    return headers;
}

std::vector<std::string> extractCompilerFlags(CompilationDatabase* db) {
    std::set<std::string> flagSet;
    std::vector<std::string> flags;
    
    auto allFiles = db->getAllFiles();
    if (allFiles.empty()) {
        return flags;
    }
    
    auto commands = db->getCompileCommands(allFiles[0]);
    if (commands.empty()) {
        return flags;
    }
    
    auto& cmdArgs = commands[0].CommandLine;
    
    for (size_t i = 0; i < cmdArgs.size(); ++i) {
        const std::string& arg = cmdArgs[i];
        
        if (arg == "-I" && i + 1 < cmdArgs.size()) {
            flagSet.insert(arg);
            flagSet.insert(cmdArgs[++i]);
        } else if (arg.starts_with("-I")) {
            flagSet.insert(arg);
        }
        else if (arg == "-isystem" && i + 1 < cmdArgs.size()) {
            flagSet.insert(arg);
            flagSet.insert(cmdArgs[++i]);
        }
        else if (arg.starts_with("-D")) {
            flagSet.insert(arg);
        }
        else if (arg.starts_with("-std=")) {
            // Convert gnu++20 to c++20
            std::string stdFlag = arg;
            if (stdFlag.find("gnu++") != std::string::npos) {
                stdFlag = "-std=c++" + stdFlag.substr(stdFlag.find("++") + 2);
            }
            flagSet.insert(stdFlag);
        }
        else if (arg == "-fPIC" || arg == "-pthread" || arg.starts_with("-m")) {
            flagSet.insert(arg);
        }
    }
    
    flags.assign(flagSet.begin(), flagSet.end());
    
    // Ensure we have C++ mode
    bool hasStd = false;
    for (const auto& flag : flags) {
        if (flag.starts_with("-std=")) {
            hasStd = true;
            break;
        }
    }
    if (!hasStd) {
        flags.push_back("-std=c++17");
    }
    
    // Add -x c++ to force C++ mode
    flags.insert(flags.begin(), "c++");
    flags.insert(flags.begin(), "-x");
    
    return flags;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <compile-commands-dir> <header-directory>\n";
        return 1;
    }

    std::string errorMessage;
    auto db = CompilationDatabase::loadFromDirectory(argv[1], errorMessage);
    
    if (!db) {
        std::cerr << "Error loading compile_commands.json: " << errorMessage << "\n";
        return 1;
    }

    auto compilerFlags = extractCompilerFlags(db.get());
    
    std::cout << "Extracted compiler flags:\n";
    for (const auto& flag : compilerFlags) {
        std::cout << "  " << flag << "\n";
    }
    
    auto headers = findHeaders(argv[2]);
    std::cout << "\nFound " << headers.size() << " headers\n\n";

    ClassPrinter Printer;
    MatchFinder Finder;
    
    Finder.addMatcher(
        cxxRecordDecl(
            isClass(),
            has(typeAliasDecl(
                hasName("__parse"),
                hasType(asString("void"))
            ))
        ).bind("class"), 
        &Printer
    );

    for (const auto& header : headers) {
        std::ifstream file(header);
        if (!file) {
            std::cerr << "Failed to read: " << header << "\n";
            continue;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string code = buffer.str();
        
        std::cout << "Parsing: " << header << "\n";
        
        runToolOnCodeWithArgs(
            newFrontendActionFactory(&Finder)->create(),
            code,
            compilerFlags,
            header
        );
    }

    return 0;
}
