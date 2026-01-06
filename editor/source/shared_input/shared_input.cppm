module;

#include <cstdint>

export module zeytin.shared_input;

export struct SharedInputState {
    // mouse state in virtual coordinates 
    float mouse_x;
    float mouse_y;
    float mouse_delta_x;
    float mouse_delta_y;
    float scroll_delta;

    // mouse buttons: 0=left, 1=right, 2=middle
    uint8_t mouse_buttons[3];        // current state: 0=up, 1=down
    uint8_t mouse_buttons_pressed[3]; // just pressed this frame
    uint8_t mouse_buttons_released[3]; // just released this frame

    // keyboard state. 512 keys to cover Raylib's full range including modifiers 340+
    uint8_t keys[512];          // current state: 0=up, 1=down
    uint8_t keys_pressed[512];  // just pressed this frame
    uint8_t keys_released[512]; // just released this frame

    // frame counter to detect new input
    uint64_t frame_counter;

    // window focus state
    uint8_t has_focus;
    uint8_t is_hovered;
};

export constexpr const char* SHARED_INPUT_NAME = "/zeytin_input";

export class SharedInputWriter {
public:
    SharedInputWriter();
    ~SharedInputWriter();

    bool initialize();
    void shutdown();

    void begin_frame();  // call at start of frame to reset pressed/released states

    void set_mouse_position(float x, float y);
    void set_mouse_delta(float dx, float dy);
    void set_scroll_delta(float delta);
    void set_mouse_button(int button, bool down);
    void set_key(int key, bool down);
    void set_focus(bool focused, bool hovered);

    void end_frame();  // call at end of frame to increment counter

    bool is_initialized() const { return m_initialized; }

private:
    bool m_initialized = false;
    int m_shm_fd = -1;
    void* m_mapped_memory = nullptr;

    // track previous state for press/release detection
    uint8_t m_prev_mouse_buttons[3] = {0};
    uint8_t m_prev_keys[512] = {0};
};

export class SharedInputReader {
public:
    SharedInputReader();
    ~SharedInputReader();

    bool initialize();
    void shutdown();

    // call once per frame to update local state and detect transitions
    void update();

    // mouse queries
    float get_mouse_x() const;
    float get_mouse_y() const;
    float get_mouse_delta_x() const;
    float get_mouse_delta_y() const;
    float get_scroll_delta() const;

    bool is_mouse_button_down(int button) const;
    bool is_mouse_button_pressed(int button) const;
    bool is_mouse_button_released(int button) const;

    // keyboard queries
    bool is_key_down(int key) const;
    bool is_key_pressed(int key) const;
    bool is_key_released(int key) const;

    // focus
    bool has_focus() const;
    bool is_hovered() const;

    bool is_initialized() const { return m_initialized; }

private:
    bool m_initialized = false;
    int m_shm_fd = -1;
    void* m_mapped_memory = nullptr;

    // local state for detecting transitions (avoids sync issues)
    uint8_t m_prev_keys[512] = {0};
    uint8_t m_prev_mouse_buttons[3] = {0};
    uint8_t m_keys_pressed[512] = {0};
    uint8_t m_keys_released[512] = {0};
    uint8_t m_mouse_pressed[3] = {0};
    uint8_t m_mouse_released[3] = {0};
    uint64_t m_last_frame = 0;
};
