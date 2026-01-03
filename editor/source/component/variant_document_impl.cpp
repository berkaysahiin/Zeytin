module;

#include <fstream>
#include <filesystem>

module zeytin.variant.document;
import zeytin.resource;

void VariantDocument::load_from_file() {
    if (m_name.empty()) {
        //log_error() << "Error: Cannot load variant with empty name" << std::endl;
        return;
    }

    std::filesystem::path path = ResourceManager::get().get_component_path(m_name);

    std::ifstream in_file(path);
    if (!in_file.is_open()) {
        //log_error() << "Error: Failed to open variant file: " << path << std::endl;
        return;
    }

    std::string json_string;
    try {
        json_string = std::string(
            std::istreambuf_iterator<char>(in_file),
            std::istreambuf_iterator<char>()
        );
    } catch (const std::exception& e) {
        //log_error() << "Error: Failed to read variant file: " << path << " - " << e.what() << std::endl;
        in_file.close();
        return;
    }

    in_file.close();

    if (json_string.empty()) {
        //log_error() << "Error: Variant file is empty: " << path << std::endl;
        return;
    }

    m_document.Parse(json_string.c_str());

    if (m_document.HasParseError()) {
        //log_error() << "Error: JSON parse error in variant file: " << path << std::endl;
        //log_error() << "  Error code: " << m_document.GetParseError() << std::endl;
        //log_error() << "  Error offset: " << m_document.GetErrorOffset() << std::endl;
        return;
    }

    if (!m_document.IsObject()) {
        //log_error() << "Error: Variant file does not contain a valid JSON object: " << path << std::endl;
        return;
    }
}
