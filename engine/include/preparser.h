#pragma once

#define PROPERTY(...) [[clang::annotate(#__VA_ARGS__)]]

#define COMPONENT(ClassName) \
    using ___component___ = void; \
public: \
    ClassName() = default; \
    ClassName(VariantCreateInfo info) : VariantBase(info) {} \
