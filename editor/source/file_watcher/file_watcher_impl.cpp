module;

#include <filesystem>
#include <map>
#include <vector>
#include <thread>
#include <unordered_map>

module zeytin.filewatcher;
import zeytin.logger;

struct FileWatcher::Impl
{ 
	Impl(fs::path path_to_watch, PollingInterval interval) :
		m_path_to_watch(path_to_watch), m_polling_interval(interval)
	{}

	std::filesystem::path m_path_to_watch;
    PollingInterval m_polling_interval;
    std::unordered_map<fs::path, fs::file_time_type> m_paths;
    bool m_running = false;
    std::thread m_watch_thread;

    std::map<std::string, std::vector<Callback>> m_callbacks;
    std::vector<Callback> m_general_callbacks;
};

FileWatcher::FileWatcher(const std::filesystem::path& path_to_watch, std::chrono::duration<int, std::milli> polling_interval)
    : pImpl(new Impl(path_to_watch, polling_interval)) {

    const bool directory = std::filesystem::exists(path_to_watch);
    if(!directory) {
        //log_error() << "Directory passed to file watcher does not exists:" << path_to_watch << std::endl;
        return;
    }

    for(auto& file : fs::recursive_directory_iterator(pImpl->m_path_to_watch)) {
        if (fs::is_regular_file(file)) {
            pImpl->m_paths[file.path()] = fs::last_write_time(file);
        }
    }
}

FileWatcher::~FileWatcher() {
    stop();
	delete pImpl;
}

void FileWatcher::add_callback(const std::vector<std::string>& extensions, Callback callback) {
    for (const auto& ext : extensions) {
        pImpl->m_callbacks[ext].push_back(callback);
    }
}

void FileWatcher::add_callback(Callback callback) {
    pImpl->m_general_callbacks.push_back(callback);
}

void FileWatcher::start() {
    pImpl->m_running = true;
    pImpl->m_watch_thread = std::thread(&FileWatcher::watch_loop, this);
}

void FileWatcher::stop() {
    pImpl->m_running = false;
    if (pImpl->m_watch_thread.joinable()) {
        pImpl->m_watch_thread.join();
    }
}

void FileWatcher::watch_loop() {
    while (pImpl->m_running) {

		// waiting until the directory we are looking is created
        const bool directory_exists = std::filesystem::exists(pImpl->m_path_to_watch); 
        if(!directory_exists) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }

		// check if any file is deleted
        auto it = pImpl->m_paths.begin();
        while (it != pImpl->m_paths.end()) {
            if (!fs::exists(it->first)) {
                notify_callbacks(it->first, FileEvent::Deleted);
                it = pImpl->m_paths.erase(it);
            } else {
                it++;
            }
        }

		// modified and created files
        for (auto& file : fs::recursive_directory_iterator(pImpl->m_path_to_watch)) {
            if (fs::is_regular_file(file)) {
                const auto& current_file_last_write_time = fs::last_write_time(file);
                const auto& path = file.path();
                if (pImpl->m_paths.find(path) != pImpl->m_paths.end()) {
                    if (pImpl->m_paths[path] != current_file_last_write_time) {
                        pImpl->m_paths[path] = current_file_last_write_time;
                        notify_callbacks(path, FileEvent::Modified);
                    }
                }
                else {
                    pImpl->m_paths[path] = current_file_last_write_time;
                    notify_callbacks(path, FileEvent::Created);
                }
            }
        }
        std::this_thread::sleep_for(pImpl->m_polling_interval);
    }
}

void FileWatcher::notify_callbacks(const std::string& path, const FileEvent fileEvent) {
    fs::path filepath(path);
    std::string extension = filepath.extension().string();
    
    if (pImpl->m_callbacks.find(extension) != pImpl->m_callbacks.end()) {
        for (const auto& callback : pImpl->m_callbacks[extension]) {
            callback(filepath, fileEvent);
        }
    }
    
    for (const auto& callback : pImpl->m_general_callbacks) {
        callback(filepath, fileEvent);
    }
}
