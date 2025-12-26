#pragma once

#include "variant/variant_base.h"
#include "raylib.h"

class Collider : public VariantBase {
    VARIANT(Collider);
    REQUIRES(Position, Scale);

public:
    float width = 100.0f; PROPERTY();
    float height = 100.0f; PROPERTY();
    
    bool show_debug = false; PROPERTY();
    
    virtual void on_init() override;
    virtual void on_update() override;
    
    Rectangle get_bounds() const;
    bool overlaps(const Rectangle& other) const;
    bool overlaps(const Collider& other) const;

private:
    void draw_debug();
};
