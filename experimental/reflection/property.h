#pragma once

#include <string>
#include <any>
#include <functional>
#include <utility>

#define ZPROPERTY(...)

class Property {
public:
    template<typename Class, typename T>
    Property(std::string name, T Class::* member)
        : name_(std::move(name))
        , getter_([member](const std::any& obj) -> std::any {
            const auto& instance = std::any_cast<const Class&>(obj);
            return instance.*member;
        })
        , setter_([member](std::any& obj, const std::any& value) {
            auto& instance = std::any_cast<Class&>(obj);
            instance.*member = std::any_cast<T>(value);
        }) {}

    [[nodiscard]]
    const std::string& name() const { return name_; }

    [[nodiscard]]
    std::any get_value(const std::any& obj) const {
        return getter_(obj);
    }

    template<typename T> [[nodiscard]]
    T get_value(const std::any& obj) const {
        return std::any_cast<T>(getter_(obj));
    }

    void set_value(std::any& obj, const std::any& value) const {
        setter_(obj, value);
    }

private:
    std::string name_;
    std::function<std::any(const std::any&)> getter_;
    std::function<void(std::any&, const std::any&)> setter_;
};