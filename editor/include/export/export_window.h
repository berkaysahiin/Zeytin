#pragma once

#include <string>
#include "entity/entity_list.h"
#include "utility/singleton.h"

class ExportWindow {
    MAKE_SINGLETON(ExportWindow);

public:
    void render();
    void set_entity_list(EntityList* entity_list) { m_entity_list = entity_list; }

private:
    ExportWindow() = default;
    void export_scene();
    
    char m_scene_name[128] = "";
    EntityList* m_entity_list = nullptr;
    bool m_show_success_popup = false;
    std::string m_success_message;
    bool m_show_error_popup = false;
    std::string m_error_message;
};
