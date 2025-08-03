#pragma once
#include <memory>
#include <string>
#include <typeindex>

#include "type.h"

class TypeRegistry {
public:
    static TypeRegistry& instance() {
        static TypeRegistry registry;
        return registry;
    }

    template<typename T>
    Type& register_type(const std::string& name) {
        auto type_id = std::type_index(typeid(T));
        auto type = std::make_unique<Type>(type_id);
        type->name_ = name;

        if constexpr (std::is_default_constructible_v<T>) {
            type->creator_ = []() -> std::any { return T{}; };
        } else {
            type->creator_ = []() -> std::any { return std::any{}; };
        }

        Type* type_ptr = type.get();
        types_[type_id] = std::move(type);
        return *type_ptr;
    }

    template<typename T>
    const Type* get_type_t(const T&) const {
        const auto type_id = std::type_index(typeid(T));
        const auto it = types_.find(type_id);
        return it != types_.end() ? it->second.get() : nullptr;
    }


   template<typename T>
   const Type* get_type_t() const {
        const auto type_id = std::type_index(typeid(T));
        const auto it = types_.find(type_id);
        return it != types_.end() ? it->second.get() : nullptr;
    }

    const Type* get_type_s(const std::string& name) const {
        for (const auto& [type_id, type] : types_) {
            if (type->name() == name) {
                return type.get();
            }
        }
        return nullptr;
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<Type>> types_;
};
