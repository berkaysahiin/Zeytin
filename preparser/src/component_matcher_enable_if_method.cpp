module;

#include "clang/AST/DeclCXX.h"
#include "clang/AST/Type.h"
#include <optional>
#include <string>

module preparser.matchers.component.enable_if_method;
import preparser.types;
import preparser.logger;

static const clang::CXXMethodDecl* find_enable_if_method(const clang::CXXRecordDecl *Record, const std::string& name)
{
	if (!Record) {
		return nullptr;
	}

	for (const auto* method : Record->methods()) {
		if (method && method->getNameAsString() == name) {
			return method;
		}
	}

	for (const auto& base : Record->bases()) {
		if (const auto* baseType = base.getType()->getAsCXXRecordDecl()) {
			if (const auto* method = find_enable_if_method(baseType, name)) {
				return method;
			}
		}
	}

	return nullptr;
}

std::optional<std::string> validate_enable_if_method(const clang::CXXRecordDecl *Record,
                                                     const PropertyInfo& property)
{
	if (!property.visibility_method) {
		return {};
	}

	const auto& method_name = *property.visibility_method;
	if (method_name.empty()) {
		log("ENABLE_IF requires a method name on component: {}", property.name);
		return {};
	}

	const auto* method = find_enable_if_method(Record, method_name);
	if (!method) {
		log("ENABLE_IF method '{}' not found on component: {}", method_name, Record->getName().str());
		return {};
	}

	if (method->param_size() != 0) {
		log("ENABLE_IF method '{}' must take no arguments on component: {}", method_name, Record->getName().str());
		return {};
	}

	if (!method->getReturnType()->isBooleanType()) {
		log("ENABLE_IF method '{}' must return bool on component: {}", method_name, Record->getName().str());
		return {};
	}

	return method_name;
}
