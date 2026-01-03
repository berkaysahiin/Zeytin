module;

#include <fstream>
#include <filesystem>

module zeytin.variant.document;
import zeytin.resource;
import zeytin.logger;

void VariantDocument::load_from_file() {
    if (m_name.empty()) {
        log_error("Error: Cannot load variant with empty name");
        return;
    }

    const std::filesystem::path path = ResourceManager::get().get_component_path(m_name);

    std::ifstream in_file(path);
    if (!in_file.is_open()) {
        log_error("Error: Failed to open variant file: {}", path.string());
        return;
    }

    std::string json_string;
    try {
        json_string = std::string(
            std::istreambuf_iterator<char>(in_file),
            std::istreambuf_iterator<char>()
        );
    } catch (const std::exception& e) {
        log_error("Error: Failed to read variant file: {}. What: {}", path.string(), e.what());
        in_file.close();
        return;
    }

    in_file.close();

    if (json_string.empty()) {
        log_error("Error: Variant file is empty: {}", path.string());
        return;
    }

    m_document.Parse(json_string.c_str());

    if (m_document.HasParseError()) {
        log_error("Error: JSON parse error in variant file: {}", path.string());
        log_error("  Error offset: {}", m_document.GetErrorOffset());
        return;
    }

    if (!m_document.IsObject()) {
        log_error("Error: Variant file does not contain a valid JSON object: {}", path.string());
        return;
    }
}
