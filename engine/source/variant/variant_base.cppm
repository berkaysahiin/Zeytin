module;

#include <cstdint>

export module zeytin.component;
import zeytin.entity;

/// Base class for components
export struct Component 
{
    virtual void on_init() {}
    virtual void on_post_init() {}
    virtual void on_play_start() {}
    virtual void on_play_late_start() {}
    virtual void on_update() {}
    virtual void on_play_update() {}
    virtual void on_play_late_update() {}

    uint64_t get_id() { return entity_id; }
    const uint64_t get_id() const { return entity_id; }

    EntityID entity_id;
    bool is_dead = false;
    bool post_inited = false;
};
