module;

#include "clang/AST/DeclCXX.h"
#include <optional>
#include <string>

export module preparser.matchers.component.enable_if_method;
import preparser.types;

export std::optional<std::string> validate_enable_if_method(const clang::CXXRecordDecl *Record,
                                                            const PropertyInfo& property);
