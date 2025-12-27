#pragma once

#include "variant/variant_base.h"
#include <vector>

struct Star {
    float x;
    float y;
    float size;
    float brightness;
};

class Background : public VariantBase {
    VARIANT(Background);

public:
    Color bg_color = {10, 10, 20, 255}; PROPERTY();
    int star_count = 100; PROPERTY();
    
    virtual void on_init() override;
    virtual void on_update() override;

private:
    std::vector<Star> m_stars;
    
    void generate_stars();
    void draw_background();
};
