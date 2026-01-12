module;

#include <cstring>
#include <cstdint>
#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"

module zeytin.engine_view;
import zeytin.engine.event;


EngineView::EngineView() {
    // allocate pixel buffer for max texture size
    m_pixel_buffer = new unsigned char[SHARED_TEXTURE_WIDTH * SHARED_TEXTURE_HEIGHT * 4];

    // create initial texture
    Image img = GenImageColor(SHARED_TEXTURE_WIDTH, SHARED_TEXTURE_HEIGHT, BLACK);
    m_texture = LoadTextureFromImage(img);
    UnloadImage(img);
    m_texture_valid = true;

    // initialize shared input
    m_input_writer.initialize();

    // try initial connection
    try_connect();
}

EngineView::~EngineView() {
    if (m_texture_valid) {
        UnloadTexture(m_texture);
        m_texture_valid = false;
    }

    delete[] m_pixel_buffer;
    m_pixel_buffer = nullptr;

    m_reader.shutdown();
    m_input_writer.shutdown();
}

void EngineView::try_connect() {
    if (!m_reader.is_initialized()) {
        m_reader.initialize();
    }
}

void EngineView::update_texture() {
    if (!m_reader.is_initialized()) {
        return;
    }

    uint32_t width, height;
    if (m_reader.read_pixels(m_pixel_buffer, width, height)) {
        // new frame available, update texture
        if (width > 0 && height > 0 && width <= SHARED_TEXTURE_WIDTH && height <= SHARED_TEXTURE_HEIGHT) {
            // check if texture size changed
            if (width != m_width || height != m_height) {
                m_width = width;
                m_height = height;

                // recreate texture with new size
                if (m_texture_valid) {
                    UnloadTexture(m_texture);
                }

                const Image img = GenImageColor(width, height, BLACK);
                m_texture = LoadTextureFromImage(img);
                UnloadImage(img);
                m_texture_valid = true;
            }

            // update texture data
            UpdateTexture(m_texture, m_pixel_buffer);
        }
    }
}

void EngineView::handle_input() {
    if (!m_input_writer.is_initialized()) {
        return;
    }

    m_input_writer.begin_frame();

    // check focus and hover state
    m_is_focused = ImGui::IsWindowFocused();
    m_is_hovered = ImGui::IsItemHovered();

    const bool allow_keyboard = m_is_focused;
    const bool allow_mouse = m_is_hovered;

    m_input_writer.set_focus(allow_keyboard, allow_mouse);

    // only process input if window is focused or hovered
    if (!allow_keyboard && !allow_mouse) {
        m_input_writer.end_frame();
        return;
    }


    // handle mouse position. transform from screen space to virtual space
    if (allow_mouse && m_image_width > 0 && m_image_height > 0) {
        ImVec2 mouse_pos = ImGui::GetMousePos();
        ImVec2 window_pos = ImGui::GetWindowPos();

        // get position relative to image
        float rel_x = mouse_pos.x - window_pos.x - m_image_x;
        float rel_y = mouse_pos.y - window_pos.y - m_image_y;

        // normalize to 0-1
        float norm_x = rel_x / m_image_width;
        float norm_y = rel_y / m_image_height;

        // clamp to valid range
        norm_x = (norm_x < 0) ? 0 : (norm_x > 1) ? 1 : norm_x;
        norm_y = (norm_y < 0) ? 0 : (norm_y > 1) ? 1 : norm_y;

        // convert to virtual coordinates (engine's coordinate space)
        const float virtual_x = norm_x * static_cast<float>(m_width);
        const float virtual_y = norm_y * static_cast<float>(m_height);

        m_input_writer.set_mouse_position(virtual_x, virtual_y);

        const ImGuiIO& io = ImGui::GetIO();

        // mouse delta
        float delta_scale_x = static_cast<float>(m_width) / m_image_width;
        float delta_scale_y = static_cast<float>(m_height) / m_image_height;
        m_input_writer.set_mouse_delta(io.MouseDelta.x * delta_scale_x, io.MouseDelta.y * delta_scale_y);

		// scroll
        m_input_writer.set_scroll_delta(io.MouseWheel);
    }
 
    // mouse buttons check when hovered
    if (allow_mouse) {
        m_input_writer.set_mouse_button(0, ImGui::IsMouseDown(ImGuiMouseButton_Left));
        m_input_writer.set_mouse_button(1, ImGui::IsMouseDown(ImGuiMouseButton_Right));
        m_input_writer.set_mouse_button(2, ImGui::IsMouseDown(ImGuiMouseButton_Middle));
    } else {
        m_input_writer.set_mouse_button(0, false);
        m_input_writer.set_mouse_button(1, false);
        m_input_writer.set_mouse_button(2, false);
    }
 
    // keyboard input when focused
    if (allow_keyboard) {


        // check common game keys - map ImGui keys to Raylib key codes
        // letters A-Z (Raylib: 65-90)
        for (int i = 0; i < 26; i++) {
            bool down = ImGui::IsKeyDown(static_cast<ImGuiKey>(ImGuiKey_A + i));
            m_input_writer.set_key(65 + i, down);  // Raylib KEY_A = 65
        }

        // numbers 0-9 (Raylib: 48-57)
        for (int i = 0; i < 10; i++) {
            bool down = ImGui::IsKeyDown(static_cast<ImGuiKey>(ImGuiKey_0 + i));
            m_input_writer.set_key(48 + i, down);  // Raylib KEY_ZERO = 48
        }

        // arrow keys
        m_input_writer.set_key(262, ImGui::IsKeyDown(ImGuiKey_RightArrow));  // KEY_RIGHT
        m_input_writer.set_key(263, ImGui::IsKeyDown(ImGuiKey_LeftArrow));   // KEY_LEFT
        m_input_writer.set_key(264, ImGui::IsKeyDown(ImGuiKey_DownArrow));   // KEY_DOWN
        m_input_writer.set_key(265, ImGui::IsKeyDown(ImGuiKey_UpArrow));     // KEY_UP

        // special keys
        m_input_writer.set_key(32, ImGui::IsKeyDown(ImGuiKey_Space));        // KEY_SPACE
        m_input_writer.set_key(256, ImGui::IsKeyDown(ImGuiKey_Escape));      // KEY_ESCAPE
        m_input_writer.set_key(257, ImGui::IsKeyDown(ImGuiKey_Enter));       // KEY_ENTER
        m_input_writer.set_key(258, ImGui::IsKeyDown(ImGuiKey_Tab));         // KEY_TAB
        m_input_writer.set_key(259, ImGui::IsKeyDown(ImGuiKey_Backspace));   // KEY_BACKSPACE
        m_input_writer.set_key(261, ImGui::IsKeyDown(ImGuiKey_Delete));      // KEY_DELETE

        // modifiers keys
        m_input_writer.set_key(340, ImGui::IsKeyDown(ImGuiKey_LeftShift));   // KEY_LEFT_SHIFT
        m_input_writer.set_key(341, ImGui::IsKeyDown(ImGuiKey_LeftCtrl));    // KEY_LEFT_CONTROL
        m_input_writer.set_key(342, ImGui::IsKeyDown(ImGuiKey_LeftAlt));     // KEY_LEFT_ALT
        m_input_writer.set_key(344, ImGui::IsKeyDown(ImGuiKey_RightShift));  // KEY_RIGHT_SHIFT
        m_input_writer.set_key(345, ImGui::IsKeyDown(ImGuiKey_RightCtrl));   // KEY_RIGHT_CONTROL
        m_input_writer.set_key(346, ImGui::IsKeyDown(ImGuiKey_RightAlt));    // KEY_RIGHT_ALT

        // functions keys F1-F12
        for (int i = 0; i < 12; i++) {
            bool down = ImGui::IsKeyDown(static_cast<ImGuiKey>(ImGuiKey_F1 + i));
            m_input_writer.set_key(290 + i, down);  // Raylib KEY_F1 = 290
        }
    }

    m_input_writer.end_frame();
}

void EngineView::render() {
    float dt = GetFrameTime();

    // try to reconnect periodically if not connected
    if (!m_reader.is_initialized()) {
        m_reconnect_timer += dt;
        if (m_reconnect_timer >= 1.0f) {
            m_reconnect_timer = 0.0f;
            try_connect();
        }
    } else {
        // check for stale frames (engine died/crashed)
        uint64_t current_frame = m_reader.get_frame_counter();
        if (current_frame != m_last_seen_frame) {
            // new frame received, reset stale timer
            m_last_seen_frame = current_frame;
            m_stale_timer = 0.0f;
        } else if (current_frame > 0) {
            // no new frame, increment stale timer
            m_stale_timer += dt;
            if (m_stale_timer >= STALE_TIMEOUT) {
                // engine is dead, reset connection
                on_engine_stopped();
            }
        }
    }

    // update texture with new frame if available
    update_texture();

    // get available size in the ImGui window
    ImVec2 content_size = ImGui::GetContentRegionAvail();

    if (!m_reader.is_initialized()) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Engine not connected");
        ImGui::Text("Start the engine to see the viewport.");
        handle_input();  // Still handle input state updates
        return;
    }

    if (!m_reader.is_connected()) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Waiting for engine...");
        handle_input();
        return;
    }

    if (!m_texture_valid || m_width == 0 || m_height == 0) {
        ImGui::Text("No frame data");
        handle_input();
        return;
    }

    // calculate scaled size maintaining aspect ratio
    float aspect = static_cast<float>(m_width) / static_cast<float>(m_height);
    float display_width = content_size.x;
    float display_height = content_size.x / aspect;

    if (display_height > content_size.y) {
        display_height = content_size.y;
        display_width = content_size.y * aspect;
    }

    // center the image
    float offset_x = (content_size.x - display_width) * 0.5f;
    float offset_y = (content_size.y - display_height) * 0.5f;

    // store image position and size for input handling
    ImVec2 cursor_pos = ImGui::GetCursorPos();
    m_image_x = cursor_pos.x + offset_x;
    m_image_y = cursor_pos.y + offset_y;
    m_image_width = display_width;
    m_image_height = display_height;

    ImGui::SetCursorPos(ImVec2(m_image_x, m_image_y));

    // display the texture
    rlImGuiImageSize(&m_texture, static_cast<int>(display_width), static_cast<int>(display_height));

    // handle input after rendering (so we can check IsItemHovered)
    handle_input();
}

bool EngineView::is_connected() const {
    return m_reader.is_initialized() && m_reader.is_connected();
}

void EngineView::on_engine_stopped() {
    // shutdown shared memory connections
    m_reader.shutdown();
    m_input_writer.shutdown();

    // clear the texture to black
    clear_texture();

    // reset the state
    m_width = 0;
    m_height = 0;
    m_reconnect_timer = 0.0f;
    m_last_seen_frame = 0;
    m_stale_timer = 0.0f;

    // reinitialize input writer for next connection
    m_input_writer.initialize();

	// raise the event
    EngineEventBus::get().publish<bool>(EngineEvent::EngineStopped, true);
}

void EngineView::clear_texture() {
    if (m_texture_valid && m_pixel_buffer) {
        // fill pixel buffer with black
        std::memset(m_pixel_buffer, 0, SHARED_TEXTURE_WIDTH * SHARED_TEXTURE_HEIGHT * 4);

        // update texture with black pixels
        UpdateTexture(m_texture, m_pixel_buffer);
    }
}
