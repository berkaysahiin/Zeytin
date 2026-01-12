module;

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/TypeBase.h"
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <filesystem>
#include <llvm/Support/FileSystem.h>

#include <cassert>

module preparser.matchers.component;
import preparser.types;
import preparser.logger;
import preparser.utility;

struct Result 
{
	bool success;
	std::vector<std::string> messages;
};

enum class PropertyType 
{
	None,
	Float,
	Int,
	Bool,
	String,
};

static PropertyType get_type(const clang::FieldDecl *Field);
static void parse_property(const clang::FieldDecl *Field, PropertyInfo& property);
static Result property_type_rules(const clang::FieldDecl *Field);
static void collect_fields_recursive(const clang::CXXRecordDecl *Record, std::vector<const clang::FieldDecl*>& fields);
static bool has_virtual_component_field(const clang::CXXRecordDecl *Record);

void ComponentMatchCallback::run(const clang::ast_matchers::MatchFinder::MatchResult& Result) {
    const auto* Record = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("component");
	if (!Record || !Record->isCompleteDefinition()) {
		log("Skipping record... {}", Record->getName().str());
		return;
	}

	bool found_errors = false;

	// Component to fill. Pushed to components vector if needs generation.
	ComponentInfo component;
	component.name = Record->getName().str();

	if (has_virtual_component_field(Record)) {
		log("Skipping virtual component... {}", component.name);
		return;
	}

	const auto& SourceManager = Result.Context->getSourceManager();
    const auto Loc = Record->getLocation();
    const auto FileID = SourceManager.getFileID(Loc);
    const auto FileEntry = SourceManager.getFileEntryRefForID(FileID);

	if (FileEntry) {
        const std::string filepath = FileEntry->getName().str();
        const auto module_name = extract_module_name(filepath);

		component.source_file = filepath;
		component.generated_code_path = this->code_path / (component.source_file.stem().string() + ".cpp");
		component.generated_component_file = this->component_path / (component.name + ".component");

        if (module_name) {
            component.module_name = *module_name;
        } else {
            log("Error: Could not extract module name for component: {}", component.name);
			found_errors = true;
        }
    } else {
        log("Error: Could not get source file for component: {}", component.name);
		found_errors = true;
    }

	// Fields for Record
	std::vector<const clang::FieldDecl*> fields;

	// Check if this file needs parsing.
	const auto source_timestmap = std::filesystem::last_write_time(component.source_file);

	if(!std::filesystem::exists(component.generated_code_path)) {
		//log("[CODE_GENERATOR] Parsing: {}. First time generating {}", component.source_file.string(), component.generated_code_path.string());
		component.requires_code_generation = true;
    	collect_fields_recursive(Record, fields);
	}
	else if(source_timestmap > std::filesystem::last_write_time(component.generated_code_path)) {
		//log("[CODE_GENERATOR] Parsing: {}. Generated code is outdated {}", component.source_file.string(), component.generated_code_path.string());
		component.requires_code_generation = true;
    	collect_fields_recursive(Record, fields);
	}
	else {
		//log("[CODE_GENERATOR] Skipping: {}. Generated code is up to date {}", component.source_file.string(), component.generated_code_path.string());
		component.requires_code_generation = false;
	}

	if(!std::filesystem::exists(component.generated_component_file)) {
		//log("[DATA_GENERATOR] Parsing: {}. First time generating {}", component.source_file.string(), component.generated_component_file.string());
		component.requires_data_generation = true;
    	collect_fields_recursive(Record, fields);
	}
	else if(source_timestmap > std::filesystem::last_write_time(component.generated_component_file)) {
		//log("[DATA_GENERATOR] Parsing: {}. Generated data is outdated {}", component.source_file.string(), component.generated_component_file.string());
		component.requires_data_generation = true;
    	collect_fields_recursive(Record, fields);
	}
	else {
		//log("[DATA_GENERATOR] Skipping: {}. Generated data is up to date {}", component.source_file.string(), component.generated_component_file.string());
		component.requires_data_generation = false;
	}

	if(!component.requires_code_generation && !component.requires_data_generation) {
		goto END;
	}

	log("Parsing {}", component.name);

	for (const auto* Field : fields) {
		if(!Field || !has_any_annotation(Field)) {
			continue;
		}
		// Handle fields that are marked as property
		// First check if type is valid
		const auto [valid, messages] = property_type_rules(Field);

		if(!valid) {
			log("Found illegal type on property: \"{}\" while parsing component: \"{}\". See below errors...", Field->getName().str(), component.name);
			for(const auto& message: messages) {
				log("\t{}", message);
			}
			found_errors = true;

			// Skip field but continue to catch more errors
			continue;
		}

		PropertyInfo property;
		parse_property(Field, property);
		component.properties.push_back(property);
	}

	if(found_errors) {
        log("Error: Errors while parsing component: {}. See logs for errors...", component.name);
	}

END:
	components.push_back(component);
}

static bool has_virtual_component_field(const clang::CXXRecordDecl *Record)
{
	if (!Record) {
		return false;
	}

	for (const auto* field : Record->fields()) {
		if (!field) {
			continue;
		}

		if (field->getName() == "___virtual_component_tag") {
			return true;
		}

		const auto type = field->getType().getCanonicalType().getUnqualifiedType();
		const auto type_name = type.getAsString();
		if (type_name == "VirtualComponentTag" || type_name == "struct VirtualComponentTag") {
			return true;
		}
	}

	return false;
}

static void collect_fields_recursive(const clang::CXXRecordDecl *Record, std::vector<const clang::FieldDecl*>& fields)
{
	if(!Record) return;

	for (const auto& base : Record->bases()) {
        if (const auto* baseType = base.getType()->getAsCXXRecordDecl()) {
            collect_fields_recursive(baseType, fields);
        }
    }

	for (const auto* Field : Record->fields()) {
        fields.push_back(Field);
    }
}

static void parse_property(const clang::FieldDecl *Field, PropertyInfo& property)
{
	assert(Field != nullptr);
	assert(has_any_annotation(Field));
	property.attrs = parse_attr_from_annotation(get_annotation_value(Field).value());
	property.annotation = get_annotation_value(Field).value();
	property.name = Field->getName().str();

	const PropertyType type = get_type(Field);
	assert(type != PropertyType::None);

	Field->hasInClassInitializer();

	const clang::Expr *init = Field->hasInClassInitializer() 
    ? Field->getInClassInitializer()->IgnoreImplicit() : nullptr;

	if (type == PropertyType::Int) {
    	int64_t value = 0; 
    	if (init) {
    	    if (const auto *lit = dyn_cast<clang::IntegerLiteral>(init)) {
    	        value = lit->getValue().getSExtValue();
    	    }
    	}
		property.value = (int)value;
	}
	else if (type == PropertyType::Float) {
    	float value = 0.0f;
    	if (init) {
    	    if (const auto *lit = dyn_cast<clang::FloatingLiteral>(init)) {
    	        value = lit->getValue().convertToFloat();
    	    } else if (const auto *lit = dyn_cast<clang::IntegerLiteral>(init)) {
    	        value = static_cast<float>(lit->getValue().getSExtValue());
    	    }
    	}
    	property.value = value;
	}
	// bool
	else if (type == PropertyType::Bool) {
	    bool value = false;  // default
	    if (init) {
	        if (const auto *lit = dyn_cast<clang::CXXBoolLiteralExpr>(init)) {
	            value = lit->getValue();
	        }
	    }
		property.value = value;
	}
	// std::string
	else if (type == PropertyType::String) {
	    std::string value = "";  // default
	    if (init) {
	        if (const auto *ctor = dyn_cast<clang::CXXConstructExpr>(init)) {
	            if (ctor->getNumArgs() > 0) {
	                if (const auto *lit = dyn_cast<clang::StringLiteral>(
	                        ctor->getArg(0)->IgnoreImplicit())) {
	                    value = lit->getString().str();
	                }
	            }
	        }
	    }
		property.value = value;
	}

}

static PropertyType get_type(const clang::FieldDecl *Field)
{
	assert(Field);

	clang::QualType type = Field->getType().getCanonicalType().getUnqualifiedType();

	if(type->isSpecificBuiltinType(clang::BuiltinType::Bool)) {
		return PropertyType::Bool;
	}

	if(type->isSpecificBuiltinType(clang::BuiltinType::Int)) {
		return PropertyType::Int;
	}

	if(type->isSpecificBuiltinType(clang::BuiltinType::Float)) {
		return PropertyType::Float;
	}

	if(type.getAsString() == "std::basic_string<char>" || type.getAsString() == "class std::basic_string<char>") {
		return PropertyType::String;
	}

	return PropertyType::None;
}

static Result property_type_rules(const clang::FieldDecl *Field)
{
	assert(Field != nullptr);

	Result result {
		.success = true
	};

	const clang::QualType qual_type = Field->getType();

	if(qual_type.isConstQualified()) {
		std::string message = "Const keyword is not allowed with properties";
		result.success = false;
		result.messages.push_back(message);
		// Not returning here to catch more errors
	}

	if(qual_type->isPointerOrReferenceType()) {
		std::string message = "Pointer or reference type is now allowed with properties";
		result.success = false;
		result.messages.push_back(message);
		// Not returning here to catch more errors
	}

	clang::QualType type = qual_type.getCanonicalType().getUnqualifiedType();

	if(type->isPointerOrReferenceType()) {
		type = type->getPointeeType();
	}

	const bool valid_type = 
		type->isBooleanType() ||
		type->isIntegerType() ||
		type->isSpecificBuiltinType(clang::BuiltinType::Float) ||
		type.getAsString() == "std::basic_string<char>" || type.getAsString() == "class std::basic_string<char>";

	if(!valid_type) {
		std::string message = std::format("Only types: float, bool, int, std::string allowed as property but found \"{}\"", type.getAsString());
		result.success = false;
		result.messages.push_back(message);
	}

	return result;
}
