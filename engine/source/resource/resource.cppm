module;

#include <string>
#include <string_view>

export module zeytin.resource;

import zeytin.resource_cache;
import zeytin.singleton;

export template<typename T>
class ResourcePtr {
public:
    ResourcePtr();
    ResourcePtr(std::string_view path);

    [[nodiscard]]
    T* get_ptr() const;

    [[nodiscard]]
    bool is_valid() const;

    [[nodiscard]] std::string_view get_path() const;
private:
    std::string m_path;
};

template<typename T>
ResourcePtr<T>::ResourcePtr() = default;

template<typename T>
ResourcePtr<T>::ResourcePtr(std::string_view path) : m_path(path) {}

template<typename T>
T* ResourcePtr<T>::get_ptr() const {
    return Singleton<ResourceCache<T>>::get().get_ptr(m_path);
}

template<typename T>
bool ResourcePtr<T>::is_valid() const {
    return get_ptr() != nullptr;
}

template<typename T>
std::string_view ResourcePtr<T>::get_path() const {
    return m_path;
}
