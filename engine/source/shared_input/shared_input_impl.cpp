module;

#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdint>

module zeytin.shared_input;

static size_t get_shm_size() {
    return sizeof(SharedInputState);
}

SharedInputWriter::SharedInputWriter() {
    std::memset(m_prev_mouse_buttons, 0, sizeof(m_prev_mouse_buttons));
    std::memset(m_prev_keys, 0, sizeof(m_prev_keys));
}

SharedInputWriter::~SharedInputWriter() {
    shutdown();
}

bool SharedInputWriter::initialize() {
    if (m_initialized) {
        return true;
    }

    m_shm_fd = shm_open(SHARED_INPUT_NAME, O_CREAT | O_RDWR, 0666);
    if (m_shm_fd == -1) {
        return false;
    }

    if (ftruncate(m_shm_fd, get_shm_size()) == -1) {
        close(m_shm_fd);
        m_shm_fd = -1;
        return false;
    }

    m_mapped_memory = mmap(nullptr, get_shm_size(), PROT_READ | PROT_WRITE, MAP_SHARED, m_shm_fd, 0);
    if (m_mapped_memory == MAP_FAILED) {
        close(m_shm_fd);
        m_shm_fd = -1;
        m_mapped_memory = nullptr;
        return false;
    }

    // initialize to zero
    std::memset(m_mapped_memory, 0, get_shm_size());

    m_initialized = true;
    return true;
}

void SharedInputWriter::shutdown() {
    if (m_mapped_memory && m_mapped_memory != MAP_FAILED) {
        munmap(m_mapped_memory, get_shm_size());
        m_mapped_memory = nullptr;
    }

    if (m_shm_fd != -1) {
        close(m_shm_fd);
        shm_unlink(SHARED_INPUT_NAME);
        m_shm_fd = -1;
    }

    m_initialized = false;
}

void SharedInputWriter::begin_frame() {
    if (!m_initialized) return;

    auto* state = static_cast<SharedInputState*>(m_mapped_memory);

    // clear pressed/released states at start of frame
    std::memset(state->mouse_buttons_pressed, 0, sizeof(state->mouse_buttons_pressed));
    std::memset(state->mouse_buttons_released, 0, sizeof(state->mouse_buttons_released));
    std::memset(state->keys_pressed, 0, sizeof(state->keys_pressed));
    std::memset(state->keys_released, 0, sizeof(state->keys_released));

    // reset deltas
    state->mouse_delta_x = 0;
    state->mouse_delta_y = 0;
    state->scroll_delta = 0;
}

void SharedInputWriter::set_mouse_position(float x, float y) {
    if (!m_initialized) return;

    auto* state = static_cast<SharedInputState*>(m_mapped_memory);
    state->mouse_x = x;
    state->mouse_y = y;
}

void SharedInputWriter::set_mouse_delta(float dx, float dy) {
    if (!m_initialized) return;

    auto* state = static_cast<SharedInputState*>(m_mapped_memory);
    state->mouse_delta_x = dx;
    state->mouse_delta_y = dy;
}

void SharedInputWriter::set_scroll_delta(float delta) {
    if (!m_initialized) return;

    auto* state = static_cast<SharedInputState*>(m_mapped_memory);
    state->scroll_delta = delta;
}

void SharedInputWriter::set_mouse_button(int button, bool down) {
    if (!m_initialized || button < 0 || button >= 3) return;

    auto* state = static_cast<SharedInputState*>(m_mapped_memory);

    uint8_t prev = m_prev_mouse_buttons[button];
    uint8_t curr = down ? 1 : 0;

    state->mouse_buttons[button] = curr;

    if (curr && !prev) {
        state->mouse_buttons_pressed[button] = 1;
    }
    if (!curr && prev) {
        state->mouse_buttons_released[button] = 1;
    }

    m_prev_mouse_buttons[button] = curr;
}

void SharedInputWriter::set_key(int key, bool down) {
    if (!m_initialized || key < 0 || key >= 512) return;

    auto* state = static_cast<SharedInputState*>(m_mapped_memory);

    uint8_t prev = m_prev_keys[key];
    uint8_t curr = down ? 1 : 0;

    state->keys[key] = curr;

    if (curr && !prev) {
        state->keys_pressed[key] = 1;
    }
    if (!curr && prev) {
        state->keys_released[key] = 1;
    }

    m_prev_keys[key] = curr;
}

void SharedInputWriter::set_focus(bool focused, bool hovered) {
    if (!m_initialized) return;

    auto* state = static_cast<SharedInputState*>(m_mapped_memory);
    state->has_focus = focused ? 1 : 0;
    state->is_hovered = hovered ? 1 : 0;
}

void SharedInputWriter::end_frame() {
    if (!m_initialized) return;

    auto* state = static_cast<SharedInputState*>(m_mapped_memory);
    state->frame_counter++;
}

SharedInputReader::SharedInputReader() {
    std::memset(m_prev_keys, 0, sizeof(m_prev_keys));
    std::memset(m_prev_mouse_buttons, 0, sizeof(m_prev_mouse_buttons));
    std::memset(m_keys_pressed, 0, sizeof(m_keys_pressed));
    std::memset(m_keys_released, 0, sizeof(m_keys_released));
    std::memset(m_mouse_pressed, 0, sizeof(m_mouse_pressed));
    std::memset(m_mouse_released, 0, sizeof(m_mouse_released));
}

SharedInputReader::~SharedInputReader() {
    shutdown();
}

bool SharedInputReader::initialize() {
    if (m_initialized) {
        return true;
    }

    m_shm_fd = shm_open(SHARED_INPUT_NAME, O_RDONLY, 0666);
    if (m_shm_fd == -1) {
        return false;
    }

    m_mapped_memory = mmap(nullptr, get_shm_size(), PROT_READ, MAP_SHARED, m_shm_fd, 0);
    if (m_mapped_memory == MAP_FAILED) {
        close(m_shm_fd);
        m_shm_fd = -1;
        m_mapped_memory = nullptr;
        return false;
    }

    m_initialized = true;
    return true;
}

void SharedInputReader::shutdown() {
    if (m_mapped_memory && m_mapped_memory != MAP_FAILED) {
        munmap(m_mapped_memory, get_shm_size());
        m_mapped_memory = nullptr;
    }

    if (m_shm_fd != -1) {
        close(m_shm_fd);
        m_shm_fd = -1;
    }

    m_initialized = false;
}

void SharedInputReader::update() {
    if (!m_initialized) return;

    auto* state = static_cast<const SharedInputState*>(m_mapped_memory);

    // clear previous frame's pressed/released states
    std::memset(m_keys_pressed, 0, sizeof(m_keys_pressed));
    std::memset(m_keys_released, 0, sizeof(m_keys_released));
    std::memset(m_mouse_pressed, 0, sizeof(m_mouse_pressed));
    std::memset(m_mouse_released, 0, sizeof(m_mouse_released));

    // detect key transitions locally
    for (int i = 0; i < 512; i++) {
        uint8_t curr = state->keys[i];
        uint8_t prev = m_prev_keys[i];

        if (curr && !prev) {
            m_keys_pressed[i] = 1;
        }
        if (!curr && prev) {
            m_keys_released[i] = 1;
        }

        m_prev_keys[i] = curr;
    }

    // detect mouse button transitions locally
    for (int i = 0; i < 3; i++) {
        uint8_t curr = state->mouse_buttons[i];
        uint8_t prev = m_prev_mouse_buttons[i];

        if (curr && !prev) {
            m_mouse_pressed[i] = 1;
        }
        if (!curr && prev) {
            m_mouse_released[i] = 1;
        }

        m_prev_mouse_buttons[i] = curr;
    }

    m_last_frame = state->frame_counter;
}

float SharedInputReader::get_mouse_x() const {
    if (!m_initialized) return 0;
    return static_cast<const SharedInputState*>(m_mapped_memory)->mouse_x;
}

float SharedInputReader::get_mouse_y() const {
    if (!m_initialized) return 0;
    return static_cast<const SharedInputState*>(m_mapped_memory)->mouse_y;
}

float SharedInputReader::get_mouse_delta_x() const {
    if (!m_initialized) return 0;
    return static_cast<const SharedInputState*>(m_mapped_memory)->mouse_delta_x;
}

float SharedInputReader::get_mouse_delta_y() const {
    if (!m_initialized) return 0;
    return static_cast<const SharedInputState*>(m_mapped_memory)->mouse_delta_y;
}

float SharedInputReader::get_scroll_delta() const {
    if (!m_initialized) return 0;
    return static_cast<const SharedInputState*>(m_mapped_memory)->scroll_delta;
}

bool SharedInputReader::is_mouse_button_down(int button) const {
    if (!m_initialized || button < 0 || button >= 3) return false;
    return static_cast<const SharedInputState*>(m_mapped_memory)->mouse_buttons[button] != 0;
}

bool SharedInputReader::is_mouse_button_pressed(int button) const {
    if (!m_initialized || button < 0 || button >= 3) return false;
    return m_mouse_pressed[button] != 0;
}

bool SharedInputReader::is_mouse_button_released(int button) const {
    if (!m_initialized || button < 0 || button >= 3) return false;
    return m_mouse_released[button] != 0;
}

bool SharedInputReader::is_key_down(int key) const {
    if (!m_initialized || key < 0 || key >= 512) return false;
    return static_cast<const SharedInputState*>(m_mapped_memory)->keys[key] != 0;
}

bool SharedInputReader::is_key_pressed(int key) const {
    if (!m_initialized || key < 0 || key >= 512) return false;
    return m_keys_pressed[key] != 0;
}

bool SharedInputReader::is_key_released(int key) const {
    if (!m_initialized || key < 0 || key >= 512) return false;
    return m_keys_released[key] != 0;
}

bool SharedInputReader::has_focus() const {
    if (!m_initialized) return false;
    return static_cast<const SharedInputState*>(m_mapped_memory)->has_focus != 0;
}

bool SharedInputReader::is_hovered() const {
    if (!m_initialized) return false;
    return static_cast<const SharedInputState*>(m_mapped_memory)->is_hovered != 0;
}
