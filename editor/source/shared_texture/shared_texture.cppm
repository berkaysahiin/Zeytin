module;

#include <cstdint>
#include <atomic>

export module zeytin.shared_texture;

// [SharedTextureHeader][Pixel Data (RGBA)]
export struct SharedTextureHeader {
    uint32_t width;
    uint32_t height;
    std::atomic<uint64_t> frame_counter;  // incremented each frame by writer
    std::atomic<uint32_t> ready;          // 1 = data ready to read, 0 = being written
};

export constexpr const char* SHARED_TEXTURE_NAME = "/zeytin_texture";
export constexpr uint32_t SHARED_TEXTURE_WIDTH = 1920;
export constexpr uint32_t SHARED_TEXTURE_HEIGHT = 1080;

export class SharedTextureWriter {
public:
    SharedTextureWriter();
    ~SharedTextureWriter();

    bool initialize();
    void shutdown();

    // write RGBA pixel data to shared memory
    void write_pixels(const unsigned char* pixels, const uint32_t width, const uint32_t height);

    bool is_initialized() const { return m_initialized; }

private:
    bool m_initialized = false;
    int m_shm_fd = -1;
    void* m_mapped_memory = nullptr;
	std::size_t m_mapped_size = 0;
};

export class SharedTextureReader {
public:
    SharedTextureReader();
    ~SharedTextureReader();

    bool initialize();
    void shutdown();

    // read pixels from shared memory into provided buffer
    // returns true if new frame available, false otherwise
    bool read_pixels(unsigned char* pixels, uint32_t& width, uint32_t& height);

    // get current frame counter
    uint64_t get_frame_counter() const;

    bool is_initialized() const { return m_initialized; }
    bool is_connected() const;

private:
    bool m_initialized = false;
    int m_shm_fd = -1;
    void* m_mapped_memory = nullptr;
	std::size_t m_mapped_size = 0;
    uint64_t m_last_frame = 0;
};
