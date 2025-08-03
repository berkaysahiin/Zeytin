#pragma once

#include <any>
#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

class Property;
class Method;

class Type {
public:
    explicit Type(const std::type_index type_id)
        : type_id_(type_id) {}

    const std::string& name() const { return name_; }

    std::any create() const {
        return creator_();
    }

    Type& property(const std::string& name, std::unique_ptr<Property> prop) {
        properties_[name] = std::move(prop);
        return *this;
    }

    Type& method(const std::string& name, std::unique_ptr<Method> mth) {
        methods_[name] = std::move(mth);
        return *this;
    }

    const Property* get_property(const std::string& name) const {
        const auto it = properties_.find(name);
        return it != properties_.end() ? it->second.get() : nullptr;
    }

    const Method* get_method(const std::string& name) const {
        const auto it = methods_.find(name);
        return it != methods_.end() ? it->second.get() : nullptr;
    }

    auto get_properties() const {
        std::vector<const Property*> props;
        props.reserve(properties_.size());
        for (const auto& [name, prop] : properties_) {
            props.push_back(prop.get());
        }
        return props;
    }

    auto get_methods() const {
        std::vector<const Method*> methods;
        methods.reserve(methods_.size());
        for (const auto& [name, method] : methods_) {
            methods.push_back(method.get());
        }
        return methods;
    }

private:
    friend class TypeRegistry;
    std::type_index type_id_;
    std::string name_;
    std::function<std::any()> creator_;
    std::unordered_map<std::string, std::unique_ptr<Property>> properties_;
    std::unordered_map<std::string, std::unique_ptr<Method>> methods_;
};