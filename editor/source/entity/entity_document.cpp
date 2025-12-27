#include "entity/entity_document.h"

#include <fstream>

#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include "logger.h"

std::string EntityDocument::as_string() const {
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    if (!m_document.Accept(writer)) {
        log_error() << "Failed to serialize JSON document" << std::endl;
        exit(1);
    }
    return buffer.GetString();
}

void EntityDocument::save_to_file(const std::filesystem::path& path) const {
	if(m_dead) return;

    std::filesystem::create_directories(path.parent_path());
    
    std::ofstream out_file(path);
    
    if (!out_file.is_open()) {
        log_error() << "Failed to open file for writing: " << path << std::endl;
        return;
    }
    
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    
    if (!m_document.Accept(writer)) {
        log_error() << "Failed to serialize JSON document" << std::endl;
        return;
    }
    
    out_file << buffer.GetString();
    
    if (out_file.fail()) {
        log_error() << "Failed to write to file: " << path << std::endl;
    }

    
    out_file.close();
}

void EntityDocument::load_from_file(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        log_error() << "File does not exist: " << path << std::endl;
        return;
    }
    
    std::ifstream in_file(path);
    
    if (!in_file.is_open()) {
        log_error() << "Failed to open file: " << path << std::endl;
        return;
    }
    
    std::string json_string((std::istreambuf_iterator<char>(in_file)), std::istreambuf_iterator<char>());
    in_file.close();
    
    m_document.Parse(json_string.c_str());
    
    if (m_document.HasParseError()) {
        log_error() << "JSON parse error at offset " << m_document.GetErrorOffset() << ": " 
                  << m_document.GetParseError() << std::endl;
    }
}
