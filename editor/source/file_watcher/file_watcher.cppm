module;

#include <chrono>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

export module zeytin.filewatcher;

export enum class FileEvent {
	Created,
	Modified,
	Deleted,
};

namespace fs = std::filesystem;

export class FileWatcher {
public:
    using Callback = std::function<void(const fs::path&, const FileEvent)>;
	using PollingInterval = std::chrono::duration<int, std::milli>;

    FileWatcher(const std::filesystem::path& path_to_watch, PollingInterval polling_interval = std::chrono::milliseconds(1000));
    ~FileWatcher();
    
    FileWatcher(const FileWatcher&) = delete;
    FileWatcher& operator=(const FileWatcher&) = delete;
    FileWatcher(FileWatcher&& other) = delete;
    FileWatcher& operator=(FileWatcher&& other) = delete;

    void add_callback(const std::vector<std::string>& extensions, Callback callback);
    void add_callback(Callback callback);

    void start();
    void stop();
private:
    void watch_loop();
    void notify_callbacks(const std::string& path, const FileEvent event);

	struct Impl;
	Impl *pImpl;
};
