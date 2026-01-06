module;

#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdint>

module zeytin.shared_texture;

// Calculate total shared memory size
static size_t calculate_shm_size() {
    return sizeof(SharedTextureHeader) + (SHARED_TEXTURE_WIDTH * SHARED_TEXTURE_HEIGHT * 4);
}

SharedTextureWriter::SharedTextureWriter() = default;

SharedTextureWriter::~SharedTextureWriter() {
    shutdown();
}

bool SharedTextureWriter::initialize() {
    if (m_initialized) {
        return true;
    }

    m_mapped_size = calculate_shm_size();

    // create shared memory object
    m_shm_fd = shm_open(SHARED_TEXTURE_NAME, O_CREAT | O_RDWR, 0666);
    if (m_shm_fd == -1) {
        return false;
    }

    // set size
    if (ftruncate(m_shm_fd, m_mapped_size) == -1) {
        close(m_shm_fd);
        m_shm_fd = -1;
        return false;
    }

    // map to memory
    m_mapped_memory = mmap(nullptr, m_mapped_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_shm_fd, 0);
    if (m_mapped_memory == MAP_FAILED) {
        close(m_shm_fd);
        m_shm_fd = -1;
        m_mapped_memory = nullptr;
        return false;
    }

    // initialize header
    auto* header = static_cast<SharedTextureHeader*>(m_mapped_memory);
    header->width = SHARED_TEXTURE_WIDTH;
    header->height = SHARED_TEXTURE_HEIGHT;
    header->frame_counter = 0;
    header->ready = 0;

    m_initialized = true;
    return true;
}

void SharedTextureWriter::shutdown() {
    if (m_mapped_memory && m_mapped_memory != MAP_FAILED) {
        munmap(m_mapped_memory, m_mapped_size);
        m_mapped_memory = nullptr;
    }

    if (m_shm_fd != -1) {
        close(m_shm_fd);
        shm_unlink(SHARED_TEXTURE_NAME);  // writer cleans up
        m_shm_fd = -1;
    }

    m_initialized = false;
}

void SharedTextureWriter::write_pixels(const unsigned char* pixels, const uint32_t width, const uint32_t height) {
    if (!m_initialized || !pixels) {
        return;
    }

    auto* header = static_cast<SharedTextureHeader*>(m_mapped_memory);
    unsigned char* pixel_data = static_cast<unsigned char*>(m_mapped_memory) + sizeof(SharedTextureHeader);

    // mark as being written
    header->ready = 0;

    // update dimensions
    header->width = width;
    header->height = height;

    // copy pixel data
    size_t data_size = width * height * 4;  // RGBA
    std::memcpy(pixel_data, pixels, data_size);

    // increment frame counter and mark as ready
    header->frame_counter++;
    header->ready = 1;
}

SharedTextureReader::SharedTextureReader() = default;

SharedTextureReader::~SharedTextureReader() {
    shutdown();
}

bool SharedTextureReader::initialize() {
    if (m_initialized) {
        return true;
    }

    m_mapped_size = calculate_shm_size();

    // open existing shared memory. don't create
    m_shm_fd = shm_open(SHARED_TEXTURE_NAME, O_RDONLY, 0666);
    if (m_shm_fd == -1) {
        return false;  // engine is not running yet
    }

    // map to memory (read-only)
    m_mapped_memory = mmap(nullptr, m_mapped_size, PROT_READ, MAP_SHARED, m_shm_fd, 0);
    if (m_mapped_memory == MAP_FAILED) {
        close(m_shm_fd);
        m_shm_fd = -1;
        m_mapped_memory = nullptr;
        return false;
    }

    m_initialized = true;
    return true;
}

void SharedTextureReader::shutdown() {
    if (m_mapped_memory && m_mapped_memory != MAP_FAILED) {
        munmap(m_mapped_memory, m_mapped_size);
        m_mapped_memory = nullptr;
    }

    if (m_shm_fd != -1) {
        close(m_shm_fd);
        // reader does NOT unlink so let writer clean up
        m_shm_fd = -1;
    }

    m_initialized = false;
}

bool SharedTextureReader::read_pixels(unsigned char* pixels, uint32_t& width, uint32_t& height) {
    if (!m_initialized || !pixels) {
        return false;
    }

    auto* header = static_cast<const SharedTextureHeader*>(m_mapped_memory);
    const unsigned char* pixel_data = static_cast<const unsigned char*>(m_mapped_memory) + sizeof(SharedTextureHeader);

    // check if data is ready and if it's a new frame
    if (header->ready == 0) {
        return false;  // Still being written
    }

    if (header->frame_counter == m_last_frame) {
        return false;  // no new frame
    }

    // read dimensions
    width = header->width;
    height = header->height;

    // copy pixel data
    size_t data_size = width * height * 4;
    std::memcpy(pixels, pixel_data, data_size);

    m_last_frame = header->frame_counter;
    return true;
}

uint64_t SharedTextureReader::get_frame_counter() const {
    if (!m_initialized) {
        return 0;
    }

    auto* header = static_cast<const SharedTextureHeader*>(m_mapped_memory);
    return header->frame_counter;
}

bool SharedTextureReader::is_connected() const {
    if (!m_initialized) {
        return false;
    }

    auto* header = static_cast<const SharedTextureHeader*>(m_mapped_memory);
    return header->frame_counter > 0;
}
