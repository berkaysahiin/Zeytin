module;

#include <string>
#include <unordered_map>
#include <filesystem>

export module zeytin.resource_cache;
import zeytin.singleton;

export template<typename T>
class ResourceCache : public Singleton<ResourceCache<T>> {
    friend class Singleton<ResourceCache<T>>;
public:
    ResourceCache(const ResourceCache&) = delete;
    ResourceCache& operator=(const ResourceCache&) = delete;
    ResourceCache(ResourceCache&&) = delete;
    ResourceCache& operator=(ResourceCache&&) = delete;

    T* get_ptr(const std::string& path);
    void reload(const std::string& path);
    void check_hot_reload();
    void clear();

private:
    ResourceCache() = default;
    ~ResourceCache() = default;

    std::unordered_map<std::string, T> m_cache;
    std::unordered_map<std::string, std::filesystem::file_time_type> m_last_modified;
};
