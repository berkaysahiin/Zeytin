#include "core/zeytin.h"
#include "remote_logger/remote_logger.h"
#include "common/property.h"

#define IGNORE_QUERIES()
#define REQUIRES(...)

#define SET_CALLBACK(callback_name) \
    void callback_name();

#define VARIANT(ClassName) \
public: \
    ClassName() = default; \
    ClassName(VariantCreateInfo info) : VariantBase(info) {} \
    RTTR_ENABLE(VariantBase); \
    static constexpr const char* get_variant_name() { return #ClassName; } \
private:
