module;

#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include <format>
#include <filesystem>
#include <stdexcept>
#include <utility>

module preparser.rttr_generator;
import preparser.types;
import preparser.logger;

void generate_rttr_registration(
    const std::vector<ComponentInfo>& components,
    const std::filesystem::path& output_path)
{
    std::vector<const ComponentInfo*> ordered_components;
    ordered_components.reserve(components.size());
    for (const auto& component : components) {
        ordered_components.push_back(&component);
    }

    std::ranges::sort(ordered_components, {}, [](const ComponentInfo* component) {
        return std::pair(component->module_name, component->name);
    });

    log("Generating: {}", output_path.string());
    std::ofstream out(output_path);
    if (!out.is_open()) {
        throw std::runtime_error(std::format("Failed to open output file: {}", output_path.string()));
    }

    out << "#include \"rttr/registration.h\"\n\n";
    std::string previous_module;
    for (const ComponentInfo* component : ordered_components) {
        if (component->module_name != previous_module) {
            out << std::format("import {};\n", component->module_name);
            previous_module = component->module_name;
        }
    }

    out << "\nRTTR_REGISTRATION\n{\n";
    for (const ComponentInfo* component : ordered_components) {
        out << std::format("\trttr::registration::class_<{}>(\"{}\")\n",
                          component->name, component->name);
        out << "\t\t(rttr::metadata(\"is_component\", true))\n";
        out << "        .constructor<>()(rttr::policy::ctor::as_object)";

        for (const auto& method : component->methods) {
            out << std::format("\n\t\t.method(\"{}\", &{}::{})",
                              method, component->name, method);
        }

        for (const auto& prop : component->properties) {
            out << std::format("\n\t\t.property(\"{}\", &{}::{})",
                              prop.name, component->name, prop.name);
        }

        out << std::format("\n\t\t.property(\"entity_id\", &{}::entity_id)"
                          "(rttr::metadata(\"is_hidden\", true));\n",
                          component->name);
    }
    out << "}\n";
}
