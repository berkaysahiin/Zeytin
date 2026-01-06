module;

#include <cstdint>
#include "raylib.h"

export module zeytin.engine_view;
import zeytin.shared_texture;
import zeytin.shared_input;

export class EngineView {
public:
    EngineView();
    ~EngineView();

    void render();

    bool is_connected() const;
    void on_engine_stopped();

private:
    void try_connect();
    void update_texture();
    void handle_input();
    void clear_texture();

    SharedTextureReader m_reader;
    SharedInputWriter m_input_writer;

    Texture2D m_texture;
    unsigned char* m_pixel_buffer = nullptr;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    bool m_texture_valid = false;
    float m_reconnect_timer = 0.0f;

    // for input handling
    float m_image_x = 0;     
    float m_image_y = 0;
    float m_image_width = 0;  
    float m_image_height = 0;
    bool m_is_focused = false;
    bool m_is_hovered = false;

    // for detecting engine death via stale frames
    uint64_t m_last_seen_frame = 0;
    float m_stale_timer = 0.0f;
    static constexpr float STALE_TIMEOUT = 0.25f;  // consider dead after seconds of no new frames
};
