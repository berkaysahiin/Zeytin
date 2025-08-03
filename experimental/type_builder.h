#pragma once
#include "type_registery.h"

template<typename T>
class TypeBuilder {
public:
    explicit TypeBuilder(Type& type) : type_(type) {}

    template<typename MemberType>
    TypeBuilder& property(const std::string& name, MemberType T::* member) {
        type_.property(name, std::make_unique<Property>(name, member));
        return *this;
    }

    template<typename Ret, typename... Args>
    TypeBuilder& method(const std::string& name, Ret(T::*method)(Args...)) {
        type_.method(name, std::make_unique<Method>(name, method));
        return *this;
    }

    template<typename Ret, typename... Args>
    TypeBuilder& method(const std::string& name, Ret(T::*method)(Args...) const) {
        type_.method(name, std::make_unique<Method>(name, method));
        return *this;
    }

private:
    Type& type_;
};

template<typename T>
TypeBuilder<T> register_class(const std::string& name) {
    auto& type = TypeRegistry::instance().register_type<T>(name);
    return TypeBuilder<T>(type);
}

template<typename T>
const Type* type_get() {
    return TypeRegistry::instance().get_type_t<T>();
}

inline const Type* type_get(const std::string& name) {
    return TypeRegistry::instance().get_type_s(name);
}