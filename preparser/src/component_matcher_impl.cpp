module;

#include "clang/AST/TypeBase.h"
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <llvm/Support/FileSystem.h>

#include <cassert>

module preparser.matchers.component;
import preparser.types;
import preparser.logger;
import preparser.utility;

static bool property_type_rules(const clang::FieldDecl *Field);

void ComponentMatchCallback::run(const clang::ast_matchers::MatchFinder::MatchResult& Result) {
    const auto* Record = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("component");
	if (!Record || !Record->isCompleteDefinition()) {
		log("Skipping record... {}", Record->getName().str());
		return;
	}

	ComponentInfo component;
	component.name = Record->getName().str();

	for (const auto* Field : Record->fields()) {
		if (has_any_annotation(Field)) {
			const bool valid = property_type_rules(Field);
			if(!valid) {
				log("Skipping field: ", Field->getName().str());
				continue;
			}

			PropertyInfo prop;
			prop.name = Field->getNameAsString();
			prop.type = Field->getType().getAsString();
			const std::string annotations = get_annotation_value(Field).value();
			prop.attrs = parse_attr_from_annotation(annotations);
			component.properties.push_back(prop);
		}
	}

	components.push_back(component);
}

static bool property_type_rules(const clang::FieldDecl *Field)
{
	assert(Field != nullptr);

	const clang::QualType qual_type = Field->getType();

	if(qual_type.isConstQualified()) {
		log("Const keyword is not allowed with properties: {}", Field->getName().str());
		return false;
	}

	const clang::Type *type = qual_type.getTypePtr();
	assert(type != nullptr);

	if(type->isPointerOrReferenceType()) {
		log("Pointer or reference type is now allowed with properties: {}", Field->getName().str());
		return false;
	}

	clang::QualType un_qual_type = qual_type.getCanonicalType().getUnqualifiedType();

	if (un_qual_type->isBooleanType())
    	return true;

	if (un_qual_type->isIntegerType() && !un_qual_type->isBooleanType())
    	return un_qual_type->isSpecificBuiltinType(clang::BuiltinType::Int);

	if (un_qual_type->isSpecificBuiltinType(clang::BuiltinType::Float))
    	return true;

	if (un_qual_type->isSpecificBuiltinType(clang::BuiltinType::Dependent))
    	return true;

	if(qual_type.getUnqualifiedType().getAsString() == "std::string") 
		return true;

	log("Allowed property types: float, int, bool, std::string but found {}", un_qual_type.getAsString());

	return true;
}
