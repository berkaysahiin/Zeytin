module;

#include "imgui.h"
#include "rlImGui.h"

#include <string>
#include <filesystem>
#include <functional>

module zeytin.asset;
import zeytin.resource;

AssetBrowser::AssetBrowser() 
    : m_root_path(ResourceManager::get().get_root_path())
    , m_current_path(m_root_path)
{
    refresh();
}

AssetBrowser::~AssetBrowser() {
    for (auto& [path, texture] : m_image_cache) {
        UnloadTexture(texture);
    }
    m_image_cache.clear();
}

void AssetBrowser::refresh() {
    m_directory_cache.clear();
    load_directory(m_root_path);
    m_current_path = m_root_path;
}

AssetType AssetBrowser::determine_asset_type(const std::string& extension) {
    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp") {
        return AssetType::Image;
    } 
    else if (extension == ".cpp" || extension == ".h" || extension == ".lua") {
        return AssetType::Script;
    }
    else if (extension == ".entity") {
        return AssetType::Entity;
    }
    else if (extension == ".component") {
        return AssetType::Variant;
    }
    else if (extension == ".scene") {
        return AssetType::Scene;
    }
    return AssetType::Other;
}

void AssetBrowser::load_directory(const fs::path& path) {
    if (!fs::exists(path) || !fs::is_directory(path)) {
        //log_error() << "Failed to load directory: " << path << std::endl;
        return;
    }

    DirectoryInfo dir_info;
    
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.is_directory()) {
                dir_info.subdirectories.push_back(entry.path());
            } 
            else if (entry.is_regular_file()) {
                FileInfo file_info;
                file_info.name = entry.path().filename().string();
                file_info.path = entry.path();
                file_info.extension = entry.path().extension().string();
                file_info.type = determine_asset_type(file_info.extension);
                
                dir_info.files.push_back(file_info);
            }
        }
        
        m_directory_cache[path] = dir_info;
    }
    catch (const std::exception& e) {
        //log_error() << "Error loading directory " << path << ": " << e.what() << std::endl;
    }
}

void AssetBrowser::render() {
    ImGui::Text("Asset Browser");
    ImGui::Separator();
    
    if (ImGui::Button("Refresh")) {
        refresh();
    }
    
    ImGui::SameLine();
    ImGui::Checkbox("Show Previews", &m_show_previews);
    
    std::string path_str = m_current_path.string();
    ImGui::Text("Path: %s", path_str.c_str());
    ImGui::Separator();
    
    float window_width = ImGui::GetContentRegionAvail().x;
    float tree_width = window_width * 0.3f;
    [[maybe_unused]] float content_width = window_width * 0.7f;
    
    ImGui::BeginChild("DirectoryTree", ImVec2(tree_width, 0), true);
    render_directory_tree();
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    ImGui::BeginChild("ContentView", ImVec2(0, 0), true);
    render_content_view();
    ImGui::EndChild();
}

void AssetBrowser::render_directory_tree() {
    if (m_directory_cache.find(m_root_path) == m_directory_cache.end()) {
        load_directory(m_root_path);
    }
    
    std::function<void(const fs::path&, int)> render_dir = [&](const fs::path& path, int depth) {
        if (m_directory_cache.find(path) == m_directory_cache.end()) {
            load_directory(path);
        }
        
        auto& dir_info = m_directory_cache[path];
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        
        if (path == m_current_path) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        
        if (depth == 0) {
            flags |= ImGuiTreeNodeFlags_DefaultOpen;
        }
        
        std::string name = (depth == 0) ? "Resources" : path.filename().string();
        
        bool node_open = ImGui::TreeNodeEx(name.c_str(), flags);
        
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            m_current_path = path;
            m_selected_path = path;
        }
        
        if (node_open) {
            if (dir_info.subdirectories.empty()) {
                ImGui::Indent();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                ImGui::TextUnformatted("(Empty)");
                ImGui::PopStyleColor();
                ImGui::Unindent();
            } else {
                for (const auto& subdir : dir_info.subdirectories) {
                    render_dir(subdir, depth + 1);
                }
            }
            ImGui::TreePop();
        }
    };
    
    render_dir(m_root_path, 0);
}

void AssetBrowser::render_content_view() {
    if (!fs::exists(m_current_path)) {
        ImGui::Text("Invalid path");
        return;
    }
    
    if (m_directory_cache.find(m_current_path) == m_directory_cache.end()) {
        load_directory(m_current_path);
    }
    
    auto& dir_info = m_directory_cache[m_current_path];
    
    if (m_current_path != m_root_path) {
        if (ImGui::Button("<-- Back")) {
            m_current_path = m_current_path.parent_path();
            m_selected_path = m_current_path;
        }
        ImGui::Separator();
    }
    
    if (m_show_previews) {
        const float thumbnail_size = 80.0f;
        const float padding = 5.0f;
        float window_width = ImGui::GetContentRegionAvail().x;
        int columns = std::max(1, (int)(window_width / (thumbnail_size + padding)));
        
        if (ImGui::BeginTable("FileGrid", columns)) {
            int item_index = 0;
            for (const auto& file : dir_info.files) {
                ImGui::TableNextColumn();
                
                ImGui::PushID(item_index++);
                
                if (file.type == AssetType::Image) {
                    if (m_image_cache.find(file.path) == m_image_cache.end()) {
                        try {
                            m_image_cache[file.path] = LoadTexture(file.path.string().c_str());
                        } catch (...) {
                            //log_error() << "Failed to load image: " << file.path << std::endl;
                        }
                    }
                    
                    auto& texture = m_image_cache[file.path];
                    if (texture.id != 0) {
                        float aspect = (float)texture.width / (float)texture.height;
                        float width = thumbnail_size;
                        float height = width / aspect;
                        
                        if (height > thumbnail_size) {
                            height = thumbnail_size;
                            width = height * aspect;
                        }
                        
                        rlImGuiImageSize(&texture, (int)width, (int)height);
                    } else {
                        ImGui::Button("?", ImVec2(thumbnail_size, thumbnail_size));
                    }
                } else {
                    ImVec4 color(0.8f, 0.8f, 0.8f, 1.0f);
                    std::string icon;
                    
                    switch (file.type) {
                        case AssetType::Script:
                            icon = "[SCR]";
                            color = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
                            break;
                        case AssetType::Entity:
                            icon = "[ENT]";
                            color = ImVec4(1.0f, 0.7f, 0.2f, 1.0f);
                            break;
                        case AssetType::Variant:
                            icon = "[VAR]";
                            color = ImVec4(0.2f, 0.7f, 1.0f, 1.0f);
                            break;
                        case AssetType::Scene:
                            icon = "[SCN]";
                            color = ImVec4(1.0f, 0.5f, 0.8f, 1.0f);
                            break;
                        default:
                            icon = "[.]";
                            break;
                    }
                    
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    std::string buttonLabel = icon + "##" + std::to_string(item_index);
                    ImGui::Button(buttonLabel.c_str(), ImVec2(thumbnail_size, thumbnail_size));
                    ImGui::PopStyleColor();
                }
                
                ImGui::TextWrapped("%s", file.name.c_str());
                
                if (ImGui::IsItemClicked()) {
                    m_selected_path = file.path;
                }
                
                ImGui::PopID(); 
            }
            ImGui::EndTable();
        }
    } else {
        int item_index = 0;
        for (const auto& file : dir_info.files) {
            ImGui::PushID(item_index++);
            
            ImVec4 color(0.8f, 0.8f, 0.8f, 1.0f);
            std::string icon;
            
            switch (file.type) {
                case AssetType::Image:
                    icon = "🖼️ ";
                    color = ImVec4(0.5f, 0.8f, 1.0f, 1.0f);
                    break;
                case AssetType::Script:
                    icon = "📄 ";
                    color = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
                    break;
                case AssetType::Entity:
                    icon = "🟠 ";
                    color = ImVec4(1.0f, 0.7f, 0.2f, 1.0f);
                    break;
                case AssetType::Variant:
                    icon = "🔵 ";
                    color = ImVec4(0.2f, 0.7f, 1.0f, 1.0f);
                    break;
                case AssetType::Scene:
                    icon = "🌐 ";
                    color = ImVec4(1.0f, 0.5f, 0.8f, 1.0f);
                    break;
                default:
                    icon = "📄 ";
                    break;
            }
            
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            if (ImGui::Selectable((icon + file.name).c_str(), m_selected_path == file.path)) {
                m_selected_path = file.path;
            }
            ImGui::PopStyleColor();
            
            ImGui::PopID(); 
        }
    }
    
    if (!m_selected_path.empty() && fs::is_regular_file(m_selected_path)) {
        ImGui::Separator();
        ImGui::Text("Selected: %s", m_selected_path.filename().string().c_str());
        
        try {
            auto file_size = fs::file_size(m_selected_path);
            std::string size_str;
            
            if (file_size < 1024) {
                size_str = std::to_string(file_size) + " B";
            } else if (file_size < 1024 * 1024) {
                size_str = std::to_string(file_size / 1024) + " KB";
            } else {
                size_str = std::to_string(file_size / (1024 * 1024)) + " MB";
            }
            
            ImGui::Text("Size: %s", size_str.c_str());
        } catch (const std::exception& e) {
            ImGui::Text("Error getting file info: %s", e.what());
        }
    }
}
