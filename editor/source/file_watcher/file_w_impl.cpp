module;

#include <filesystem>
#include <vector>
#include <thread>

module zeytin.filewatcher;

FileW::FileW(const std::filesystem::path& path_to_watch, std::chrono::duration<int, std::milli> polling_interval)
    : m_path_to_watch(path_to_watch), m_polling_interval(polling_interval), m_running(false) {

    const bool directory = std::filesystem::exists(path_to_watch);
    if(!directory) {
        //log_error() << "Directory passed to file watcher does not exists:" << path_to_watch << std::endl;
        return;
    }

    for(auto& file : fs::recursive_directory_iterator(m_path_to_watch)) {
        if (fs::is_regular_file(file)) {
            m_paths[file.path().string()] = fs::last_write_time(file);
        }
    }
}

FileW::~FileW() {
    stop();
}

FileW::FileW(FileW&& other) noexcept
    : m_path_to_watch(std::move(other.m_path_to_watch)),
      m_polling_interval(other.m_polling_interval),
      m_paths(std::move(other.m_paths)),
      m_callbacks(std::move(other.m_callbacks)),
      m_general_callbacks(std::move(other.m_general_callbacks)),
      m_running(other.m_running),
      m_watch_thread() 
{
    if (other.m_running) {
        other.m_running = false; 
        if (other.m_watch_thread.joinable()) {
            other.m_watch_thread.join(); 
        }
        start(); 
    }
}

FileW& FileW::operator=(FileW&& other) noexcept {
    if (this != &other) {
        stop();
        
        m_path_to_watch = std::move(other.m_path_to_watch);
        m_polling_interval = other.m_polling_interval;
        m_paths = std::move(other.m_paths);
        m_callbacks = std::move(other.m_callbacks);
        m_general_callbacks = std::move(other.m_general_callbacks);
        
        bool was_running = other.m_running;
        if (was_running) {
            other.m_running = false;
            if (other.m_watch_thread.joinable()) {
                other.m_watch_thread.join();
            }
            start(); 
        }
    }
    return *this;
}

void FileW::add_callback(const std::vector<std::string>& extensions, Callback callback) {
    for (const auto& ext : extensions) {
        m_callbacks[ext].push_back(callback);
    }
}

void FileW::add_callback(Callback callback) {
    m_general_callbacks.push_back(callback);
}

void FileW::start() {
    m_running = true;
    m_watch_thread = std::thread(&FileW::watch_loop, this);
}

void FileW::stop() {
    m_running = false;
    if (m_watch_thread.joinable()) {
        m_watch_thread.join();
    }
}

void FileW::watch_loop() {
    while (m_running) {

        const bool directory_exists = std::filesystem::exists(m_path_to_watch); // waiting until the directory we are looking is created
        if(!directory_exists) {
            continue;
        }

        auto it = m_paths.begin();
        while (it != m_paths.end()) {
            if (!fs::exists(it->first)) {
                notify_callbacks(it->first, "deleted");
                it = m_paths.erase(it);
            } else {
                it++;
            }
        }
        for (auto& file : fs::recursive_directory_iterator(m_path_to_watch)) {
            if (fs::is_regular_file(file)) {
                auto current_file_last_write_time = fs::last_write_time(file);
                std::string path = file.path().string();
                if (m_paths.find(path) != m_paths.end()) {
                    if (m_paths[path] != current_file_last_write_time) {
                        m_paths[path] = current_file_last_write_time;
                        notify_callbacks(path, "modified");
                    }
                }
                else {
                    m_paths[path] = current_file_last_write_time;
                    notify_callbacks(path, "created");
                }
            }
        }
        std::this_thread::sleep_for(m_polling_interval);
    }
}

void FileW::notify_callbacks(const std::string& path, const std::string& status) {
    fs::path filepath(path);
    std::string extension = filepath.extension().string();
    
    if (m_callbacks.find(extension) != m_callbacks.end()) {
        for (const auto& callback : m_callbacks[extension]) {
            callback(filepath, status);
        }
    }
    
    for (const auto& callback : m_general_callbacks) {
        callback(filepath, status);
    }
}
