#pragma once

#include "variant/variant_base.h"
#include "raylib.h"

class ObjectiveArrow : public VariantBase {
    VARIANT(ObjectiveArrow);

public:
    float arrow_size = 40.0f; PROPERTY();
    float arrow_height = 100.0f; PROPERTY();
    float bounce_amount = 10.0f; PROPERTY();
    float bounce_speed = 3.0f; PROPERTY();

    Color arrow_color = {255, 255, 0, 200};
    
    virtual void on_init() override;
    virtual void on_update() override;

private:
    void draw_arrow();
    void draw_arrow_at(float x, float y);
    
    float m_bounce_timer = 0.0f;
};
