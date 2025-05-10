#pragma once

#include <vector>
#include <functional>
#include <type_traits>
#include <optional>

#include "core/zeytin.h"
#include "rttr/variant.h"
#include "entity/entity.h"
#include "variant/variant_base.h"

namespace Query {

inline entity_id create_entity() {
    return Zeytin::get().new_entity();
}

template<typename T>
bool has(entity_id id) {
    static_assert(std::is_base_of<VariantBase, T>::value, "T must derive from VariantBase");
    auto& variants = Zeytin::get().get_variants(id);
    
    for (const auto& variant : variants) {
        if (variant.get_type() == rttr::type::get<T>()) {
            return true;
        }
    }
    
    return false;
}

template<typename T>
bool has(const VariantBase* base) {
    return has<T>(base->entity_id);
}

template<typename T>
bool has(VariantBase* base) {
    return has<T>(base->entity_id);
}

template<typename T1, typename T2, typename... Rest>
bool has(entity_id id) {
    return has<T1>(id) && has<T2, Rest...>(id);
}

template<typename T1, typename T2, typename... Rest>
bool has(const VariantBase* base) {
    return has<T1, T2, Rest...>(base->entity_id);
}


template<typename T>
T& get(entity_id id) {
    static_assert(std::is_base_of<VariantBase, T>::value, "T must derive from VariantBase");
    auto& variants = Zeytin::get().get_variants(id);

    for (auto& variant : variants) {
        if (variant.get_type() == rttr::type::get<T>()) {
            return variant.get_value<T&>();
        }
    }

    throw std::runtime_error("Component not found despite has() check");
}

template<typename T>
T& get(const VariantBase* base) {
    return get<T>(base->entity_id);
}

template<typename T1, typename T2, typename... Rest>
std::tuple<T1&, T2&, Rest&...> get(entity_id id) {
    return std::tie(get<T1>(id), get<T2>(id), get<Rest>(id)...);
}

template<typename T1, typename T2, typename... Rest>
std::tuple<T1&, T2&, Rest&...> get(const VariantBase* base) {
    return get<T1, T2, Rest...>(base->entity_id);
}

template<typename T>
std::optional<std::reference_wrapper<T>> try_get(entity_id id) {
    if (has<T>(id)) {
        return std::optional<std::reference_wrapper<T>>(std::ref(get<T>(id)));
    }
    return std::nullopt;
}

template<typename T>
std::optional<std::reference_wrapper<T>> try_get(const VariantBase* base) {
    return try_get<T>(base->entity_id);
}

template<typename T>
const T& read(entity_id id) {
    static_assert(std::is_base_of<VariantBase, T>::value, "T must derive from VariantBase");
    return get<T>(id);
}

template<typename T>
const T& read(const VariantBase* base) {
    return read<T>(base->entity_id);
}

template<typename T1, typename T2, typename... Rest>
std::tuple<const T1&, const T2&, const Rest&...> read(entity_id id) {
    return std::tie(read<T1>(id), read<T2>(id), read<Rest>(id)...);
}

template<typename T1, typename T2, typename... Rest>
std::tuple<const T1&, const T2&, const Rest&...> read(const VariantBase* base) {
    return read<T1, T2, Rest...>(base->entity_id);
}

template<typename T>
std::optional<std::reference_wrapper<T>> try_find_first() {
    static_assert(std::is_base_of<VariantBase, T>::value, "T must derive from VariantBase");
    const rttr::type& type = rttr::type::get<T>();
    
    for (auto& [entity_id, variants] : Zeytin::get().get_storage()) {
        for (auto& variant : variants) {
            if (variant.get_type() == type) {
                return std::ref(variant.get_value<T&>());
            }
        }
    }
    
    return std::nullopt;
}

template<typename T>
T& find_first() {
    static_assert(std::is_base_of<VariantBase, T>::value, "T must derive from VariantBase");
    const rttr::type& type = rttr::type::get<T>();
    
    for (auto& [entity_id, variants] : Zeytin::get().get_storage()) {
        for (auto& variant : variants) {
            if (variant.get_type() == type) {
                return variant.get_value<T&>();
            }
        }
    }
    
    throw std::runtime_error("Not able to find_first: " + type.get_name().to_string()); 
}

template<typename T>
std::vector<std::reference_wrapper<T>> find_all() {
    static_assert(std::is_base_of<VariantBase, T>::value, "T must derive from VariantBase");
    std::vector<std::reference_wrapper<T>> results;
    const rttr::type& type = rttr::type::get<T>();
    
    for (auto& [entity_id, variants] : Zeytin::get().get_storage()) {
        for (auto& variant : variants) {
            if (variant.get_type() == type) {
                T& component = variant.get_value<T&>();
                results.push_back(std::ref(component));
            }
        }
    }
    
    return results;
}

template<typename T, typename... Rest>
std::vector<entity_id> find_all_with() {
    static_assert(std::is_base_of<VariantBase, T>::value, "T must derive from VariantBase");
    std::vector<entity_id> results;
    
    for (auto& [entity_id, variants] : Zeytin::get().get_storage()) {
        if (has<T, Rest...>(entity_id)) {
            results.push_back(entity_id);
        }
    }
    
    return results;
}

template<typename T>
std::vector<std::reference_wrapper<T>> find_where(std::function<bool(T&)> predicate) {
    static_assert(std::is_base_of<VariantBase, T>::value, "T must derive from VariantBase");
    std::vector<std::reference_wrapper<T>> results;
    const rttr::type& type = rttr::type::get<T>();
    
    for (auto& [entity_id, variants] : Zeytin::get().get_storage()) {
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


template<typename T, typename... Rest>
bool has_types(const std::vector<rttr::variant>& variants) {
    static_assert(std::is_base_of<VariantBase, T>::value, "T must derive from VariantBase");
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

template<typename T>
size_t count() {
    static_assert(std::is_base_of<VariantBase, T>::value, "T must derive from VariantBase");
    size_t count = 0;
    const rttr::type& type = rttr::type::get<T>();
    
    for (auto& [entity_id, variants] : Zeytin::get().get_storage()) {
        for (auto& variant : variants) {
            if (variant.get_type() == type) {
                count++;
                break;
            }
        }
    }
    
    return count;
}

template<typename T>
void for_each(std::function<void(T&)> action) {
    static_assert(std::is_base_of<VariantBase, T>::value, "T must derive from VariantBase");
    const rttr::type& type = rttr::type::get<T>();
    
    for (auto& [entity_id, variants] : Zeytin::get().get_storage()) {
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

template<typename T>
void remove_variant_from(entity_id id) {
    Zeytin::get().remove_variant(id, rttr::type::get<T>());
}

template<typename T>
void remove_variant_from(const VariantBase* base) {
    Zeytin::get().remove_variant(base->entity_id, rttr::type::get<T>());
}

template<typename T, typename... Args>
std::optional<std::reference_wrapper<T>> add(entity_id id, Args&&... args) {
    static_assert(std::is_base_of<VariantBase, T>::value, "T must derive from VariantBase");
    if(has<T>(id)) {
        log_warning() << "Trying to add duplicate variants to entity" << std::endl;
        return std::nullopt;
    }

    T variant(std::forward<Args>(args)...);
    variant.entity_id = id;
    variant.on_init();
    
    auto& variants = Zeytin::get().get_variants(id);
    variants.push_back(std::move(variant));

    return std::ref(Query::get<T>(id));
}

template<typename T, typename... Args>
std::optional<std::reference_wrapper<T>> add(VariantBase* base, Args&&... args) {
    return add<T>(base->entity_id, std::forward<Args>(args)...);
}

inline void remove_entity(entity_id id) {
    Zeytin::get().remove_entity(id);
}

inline bool entity_exists(entity_id id) {
    return Zeytin::get().get_storage().find(id) != Zeytin::get().get_storage().end();
}

} 
