module;

#include <chrono>
#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

export module zeytin.filewatcher;

export namespace fs = std::filesystem;

export class FileW {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileW(const std::filesystem::path& path_to_watch, 
          std::chrono::duration<int, std::milli> polling_interval = std::chrono::milliseconds(1000));
    
    ~FileW();
    
    FileW(const FileW&) = delete;
    FileW& operator=(const FileW&) = delete;
    
    FileW(FileW&& other) noexcept;
    FileW& operator=(FileW&& other) noexcept;

    void add_callback(const std::vector<std::string>& extensions, Callback callback);
    void add_callback(Callback callback);
    void start();
    void stop();

    std::filesystem::path m_path_to_watch;
    std::chrono::duration<int, std::milli> m_polling_interval;
    std::unordered_map<std::string, fs::file_time_type> m_paths;
    std::map<std::string, std::vector<Callback>> m_callbacks;
    std::vector<Callback> m_general_callbacks;
    bool m_running;
    std::thread m_watch_thread;

    void watch_loop();
    void notify_callbacks(const std::string& path, const std::string& status);
};
