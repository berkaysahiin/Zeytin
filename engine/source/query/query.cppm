module;

#include <functional>
#include <type_traits>
#include <optional>
#include <sstream>

#include "rttr/variant.h"

export module zeytin.query;
import zeytin.entity;
import zeytin.component;
import zeytin.zeytin;

namespace Query {

inline EntityID create_entity() {
    return Zeytin::get().new_entity();
}

export template<typename T>
bool has(EntityID id) {
    static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
    auto& variants = Zeytin::get().get_components(id);
    
    for (const auto& variant : variants) {
        if (variant.get_type() == rttr::type::get<T>()) {
            return true;
        }
    }
    
    return false;
}

export template<typename T>
bool has(const Component* base) {
    return has<T>(base->entity_id);
}

export template<typename T>
bool has(Component* base) {
    return has<T>(base->entity_id);
}

export template<typename T1, typename T2, typename... Rest>
bool has(EntityID id) {
    return has<T1>(id) && has<T2, Rest...>(id);
}

export template<typename T1, typename T2, typename... Rest>
bool has(const Component* base) {
    return has<T1, T2, Rest...>(base->entity_id);
}


export template<typename T>
T& get(EntityID id) {
    static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
    auto& variants = Zeytin::get().get_components(id);

    for (auto& variant : variants) {
        if (variant.get_type() == rttr::type::get<T>()) {
            return variant.get_value<T&>();
        }
    }

    std::ostringstream error_msg;
    error_msg << "Component " << typeid(T).name() << " not found on entity: " << id << std::endl;
    throw std::runtime_error(error_msg.str());
}

export template<typename T>
T& get(const Component* base) {
    return get<T>(base->entity_id);
}

export template<typename T1, typename T2, typename... Rest>
std::tuple<T1&, T2&, Rest&...> get(EntityID id) {
    return std::tie(get<T1>(id), get<T2>(id), get<Rest>(id)...);
}

export template<typename T1, typename T2, typename... Rest>
std::tuple<T1&, T2&, Rest&...> get(const Component* base) {
    return get<T1, T2, Rest...>(base->entity_id);
}

export template<typename T>
std::optional<std::reference_wrapper<T>> try_get(EntityID id) {
    if (has<T>(id)) {
        return std::optional<std::reference_wrapper<T>>(std::ref(get<T>(id)));
    }
    return std::nullopt;
}

export template<typename T>
std::optional<std::reference_wrapper<T>> try_get(const Component* base) {
    return try_get<T>(base->entity_id);
}

export template<typename T>
const T& read(EntityID id) {
    static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
    return get<T>(id);
}

export template<typename T>
const T& read(const Component* base) {
    return read<T>(base->entity_id);
}

export template<typename T1, typename T2, typename... Rest>
std::tuple<const T1&, const T2&, const Rest&...> read(EntityID id) {
    return std::tie(read<T1>(id), read<T2>(id), read<Rest>(id)...);
}

export template<typename T1, typename T2, typename... Rest>
std::tuple<const T1&, const T2&, const Rest&...> read(const Component* base) {
    return read<T1, T2, Rest...>(base->entity_id);
}

export template<typename T>
std::optional<std::reference_wrapper<T>> try_find_first() {
    static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
    const rttr::type& type = rttr::type::get<T>();
    
    for (auto& [EntityID, variants] : Zeytin::get().get_storage()) {
        for (auto& variant : variants) {
            if (variant.get_type() == type) {
                return std::ref(variant.get_value<T&>());
            }
        }
    }
    
    return std::nullopt;
}

export template<typename T>
T& find_first() {
    static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
    const rttr::type& type = rttr::type::get<T>();
    
    for (auto& [EntityID, variants] : Zeytin::get().get_storage()) {
        for (auto& variant : variants) {
            if (variant.get_type() == type) {
                return variant.get_value<T&>();
            }
        }
    }
    
    throw std::runtime_error("Not able to find_first: " + type.get_name().to_string()); 
}

export template<typename T>
std::vector<std::reference_wrapper<T>> find_all() {
    static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
    std::vector<std::reference_wrapper<T>> results;
    const rttr::type& type = rttr::type::get<T>();
    
    for (auto& [EntityID, variants] : Zeytin::get().get_storage()) {
        for (auto& variant : variants) {
            if (variant.get_type() == type) {
                T& component = variant.get_value<T&>();
                results.push_back(std::ref(component));
            }
        }
    }
    
    return results;
}

export template<typename T, typename... Rest>
std::vector<EntityID> find_all_with() {
    static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
    std::vector<EntityID> results;
    
    for (auto& [EntityID, variants] : Zeytin::get().get_storage()) {
        if (has<T, Rest...>(EntityID)) {
            results.push_back(EntityID);
        }
    }
    
    return results;
}

export template<typename T>
std::vector<std::reference_wrapper<T>> find_where(std::function<bool(T&)> predicate) {
    static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
    std::vector<std::reference_wrapper<T>> results;
    const rttr::type& type = rttr::type::get<T>();
    
    for (auto& [EntityID, variants] : Zeytin::get().get_storage()) {
        for (auto& variant : variants) {
            if (variant.get_type() == type) {
                T& component = variant.get_value<T&>();
                if (predicate(component)) {
                    results.push_back(std::ref(component));
                }
            }
        }
    }
    
    return results;
}


export template<typename T, typename... Rest>
bool has_types(const std::vector<rttr::variant>& variants) {
    static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
    bool has_t = false;
    for (const auto& variant : variants) {
        if (variant.get_type() == rttr::type::get<T>()) {
            has_t = true;
            break;
        }
    }
    
    if (!has_t) {
        return false;
    }
    
    if constexpr (sizeof...(Rest) > 0) {
        return has_types<Rest...>(variants);
    }
    
    return true;
}

export template<typename T>
size_t count() {
    static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
    size_t count = 0;
    const rttr::type& type = rttr::type::get<T>();
    
    for (auto& [EntityID, variants] : Zeytin::get().get_storage()) {
        for (auto& variant : variants) {
            if (variant.get_type() == type) {
                count++;
                break;
            }
        }
    }
    
    return count;
}

export template<typename T>
void for_each(std::function<void(T&)> action) {
    static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
    const rttr::type& type = rttr::type::get<T>();
    
    for (auto& [EntityID, variants] : Zeytin::get().get_storage()) {
        for (auto& variant : variants) {
            if (variant.get_type() == type) {
                T& component = variant.get_value<T&>();
                if(!component.is_dead) {
                    action(component);
                }
            }
        }
    }
}

export template<typename T>
void remove_variant_from(EntityID id) {
    Zeytin::get().remove_variant(id, rttr::type::get<T>());
}

export template<typename T>
void remove_variant_from(const Component* base) {
    Zeytin::get().remove_variant(base->entity_id, rttr::type::get<T>());
}

export template<typename T, typename... Args>
std::optional<std::reference_wrapper<T>> add(EntityID id, Args&&... args) {
    static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
    if(has<T>(id)) {
        //log_warning() << "Trying to add duplicate variants to entity" << std::endl;
        return std::nullopt;
    }

    T variant(std::forward<Args>(args)...);
    variant.EntityID = id;
    variant.on_init();
    
    auto& variants = Zeytin::get().get_components(id);
    variants.push_back(std::move(variant));

    return std::ref(Query::get<T>(id));
}

export template<typename T, typename... Args>
std::optional<std::reference_wrapper<T>> add(Component* base, Args&&... args) {
    return add<T>(base->entity_id, std::forward<Args>(args)...);
}

inline void remove_entity(EntityID id) {
    Zeytin::get().remove_entity(id);
}

inline bool entity_exists(EntityID id) {
    return Zeytin::get().get_storage().find(id) != Zeytin::get().get_storage().end();
}

} 
