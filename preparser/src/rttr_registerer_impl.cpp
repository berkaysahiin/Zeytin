module;

#include <string>
#include <vector>
#include <fstream>
#include <format>
#include <filesystem>

module preparser.rttr_generator;
import preparser.types;
import preparser.logger;

static std::string to_snake_case(const std::string& name) {
    std::string result;
    for (size_t i = 0; i < name.size(); ++i) {
        char c = name[i];
        if (std::isupper(c) && i > 0 && std::islower(name[i-1])) {
            result += '_';
        }
        result += std::tolower(c);
    }
    return result;
}

void generate_rttr_registration(const std::vector<ComponentInfo>& components) {
    // Generate a separate .cpp file for each component

	// Figure out the parent folder
	std::filesystem::path folder;

    for (const auto& component : components) {
		if (!component.requires_code_generation) {
			log("Up to date: {}", component.generated_code_path.string());
			continue;
		}

		log("Generating: {}", component.generated_code_path.string());

        const std::string filepath = component.generated_code_path;
		folder = component.generated_code_path.parent_path();

        std::ofstream out(filepath);
        if (!out.is_open()) {
            throw std::runtime_error(std::format("Failed to open output file: {}", filepath));
        }

        out << "#include \"rttr/registration.h\"\n\n";
        out << std::format("import {};\n", component.module_name);

        out << "\nRTTR_REGISTRATION\n{\n";

        out << std::format("\trttr::registration::class_<{}>(\"{}\")\n",
                          component.name, component.name);
        out << "\t\t(rttr::metadata(\"is_component\", true))\n";
        out << "        .constructor<>()(rttr::policy::ctor::as_object)";

        for (const auto& prop : component.properties) {
            out << std::format("\n\t\t.property(\"{}\", &{}::{})",
                              prop.name, component.name, prop.name);
        }

        out << std::format("\n\t\t.property(\"entity_id\", &{}::entity_id)"
                          "(rttr::metadata(\"is_hidden\", true));\n",
                          component.name);

        out << "}\n";
        out.close();
    }
    
    // Also generate the base Component registration file
    std::string base_filepath = std::filesystem::path(folder) / "component_rttr.cpp";
    std::ofstream base_out(base_filepath);
    if (!base_out.is_open()) {
        throw std::runtime_error(std::format("Failed to open output file: {}", base_filepath));
    }
    
    base_out << "#include \"rttr/registration.h\"\n\n";
    base_out << "import zeytin.component;\n\n";
    base_out << "RTTR_REGISTRATION\n{\n";
    base_out << "    rttr::registration::class_<Component>(\"Component\")\n";
    base_out << "        .constructor<>()(rttr::policy::ctor::as_object);\n";
    base_out << "}\n";
    base_out.close();
}
