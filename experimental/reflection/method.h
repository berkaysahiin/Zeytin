#pragma once
#include <any>
#include <functional>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

class Method {
public:
    template<typename Class, typename Ret, typename... Args>
    Method(std::string name, Ret(Class::*method)(Args...))
        : name_(std::move(name))
        , invoker_([method](std::any& obj, const std::vector<std::any>& args) -> std::any {
            auto& instance = std::any_cast<Class&>(obj);
            if constexpr (sizeof...(Args) == 0) {
                if constexpr (std::is_void_v<Ret>) {
                    (instance.*method)();
                    return std::any{};
                } else {
                    return (instance.*method)();
                }
            } else {
                return invoke_with_args(instance, method, args, std::index_sequence_for<Args...>{});
            }
        })
        , const_invoker_([](const std::any& obj, const std::vector<std::any>& args) -> std::any {
            throw std::runtime_error("Cannot call non-const method on const object");
        })
        , is_const_method_(false) {}

    template<typename Class, typename Ret, typename... Args>
    Method(std::string name, Ret(Class::*method)(Args...) const)
        : name_(std::move(name))
        , invoker_([method](std::any& obj, const std::vector<std::any>& args) -> std::any {
            auto& instance = std::any_cast<Class&>(obj);
            if constexpr (sizeof...(Args) == 0) {
                if constexpr (std::is_void_v<Ret>) {
                    (instance.*method)();
                    return std::any{};
                } else {
                    return (instance.*method)();
                }
            } else {
                return invoke_with_args_const(instance, method, args, std::index_sequence_for<Args...>{});
            }
        })
        , const_invoker_([method](const std::any& obj, const std::vector<std::any>& args) -> std::any {
            const auto& instance = std::any_cast<const Class&>(obj);
            if constexpr (sizeof...(Args) == 0) {
                if constexpr (std::is_void_v<Ret>) {
                    (instance.*method)();
                    return std::any{};
                } else {
                    return (instance.*method)();
                }
            } else {
                return invoke_with_args_const(instance, method, args, std::index_sequence_for<Args...>{});
            }
        })
        , is_const_method_(true) {}

    [[nodiscard]]
    const std::string& name() const { return name_; }

    std::any invoke(std::any& obj, const std::vector<std::any>& args = {}) const {
        return invoker_(obj, args);
    }

    [[nodiscard]]
    std::any invoke(const std::any& obj, const std::vector<std::any>& args = {}) const {
        return const_invoker_(obj, args);
    }

    [[nodiscard]]
    bool is_const_method() const { return is_const_method_; }

private:
    template<typename Class, typename Ret, typename... Args, std::size_t... I>
    static std::any invoke_with_args(Class& instance, Ret(Class::*method)(Args...),
                                   const std::vector<std::any>& args, std::index_sequence<I...>) {
        if constexpr (std::is_void_v<Ret>) {
            (instance.*method)(std::any_cast<Args>(args[I])...);
            return std::any{};
        } else {
            return (instance.*method)(std::any_cast<Args>(args[I])...);
        }
    }

    template<typename Class, typename Ret, typename... Args, std::size_t... I>
    static std::any invoke_with_args_const(const Class& instance, Ret(Class::*method)(Args...) const,
                                         const std::vector<std::any>& args, std::index_sequence<I...>) {
        if constexpr (std::is_void_v<Ret>) {
            (instance.*method)(std::any_cast<Args>(args[I])...);
            return std::any{};
        } else {
            return (instance.*method)(std::any_cast<Args>(args[I])...);
        }
    }

    std::string name_;
    std::function<std::any(std::any&, const std::vector<std::any>&)> invoker_;
    std::function<std::any(const std::any&, const std::vector<std::any>&)> const_invoker_;
    bool is_const_method_;
};