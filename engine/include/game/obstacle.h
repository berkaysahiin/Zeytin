#pragma once

#include "variant/variant_base.h"
#include "raylib.h"

class Obstacle : public VariantBase {
    VARIANT(Obstacle);
    REQUIRES(Position, Scale, Collider);

public:
    bool show_outline = true; PROPERTY();
    float outline_thickness = 2.0f; PROPERTY();
    
    virtual void on_init() override;
    virtual void on_update() override;

private:
    void draw_obstacle();
    
    Color fill_color;
    Color outline_color;
};
