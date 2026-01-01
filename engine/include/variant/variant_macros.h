#pragma once

#define PROPERTY() 
#define IGNORE_QUERIES()
#define REQUIRES(...)

#define SET_CALLBACK(callback_name) \
    void callback_name();

#define VARIANT(ClassName) \
public: \
    ClassName() = default; \
    ClassName(VariantCreateInfo info) : VariantBase(info) {} \
    static constexpr const char* get_variant_name() { return #ClassName; } \
private:

#define REGISTER(CLASS_NAME) VARIANT(CLASS_NAME)
