#pragma once

#include "rttr/rttr_enable.h"
#include "variant/variant_macros.h"
#include <functional>
#include "core/raylib_wrapper.h"

struct VariantCreateInfo {
    entity_id entity_id;

    RTTR_ENABLE();
};

struct VariantBase {
    VariantBase() = default;
    VariantBase(VariantCreateInfo info) : entity_id(info.entity_id) {}

    virtual void on_init() {}
    virtual void on_post_init() {}
    virtual void on_update() {}
    virtual void on_play_start() {}
    virtual void on_play_late_start() {}
    virtual void on_play_update() {}
    virtual void on_play_late_update() {}

    virtual bool check_dependencies(const std::string& method_name) const { return true; }

    uint64_t get_id() { return entity_id; }
    const uint64_t get_id() const { return entity_id; }

    entity_id entity_id;
    bool is_dead = false;
    bool post_inited = false;

    RTTR_ENABLE();
};

