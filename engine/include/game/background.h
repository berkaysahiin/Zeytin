#pragma once

#include "variant/variant_base.h"
#include <vector>

struct Star {
    float x;
    float y;
    float size;
    float brightness;
    float speed;  
};

class Background : public VariantBase {
    VARIANT(Background);

public:
    Color bg_color = {15, 20, 40, 255}; PROPERTY();  
    int star_count = 150; PROPERTY();
    float star_speed_min = 10.0f; PROPERTY();
    float star_speed_max = 50.0f; PROPERTY();
    
    virtual void on_init() override;
    virtual void on_update() override;

private:
    std::vector<Star> m_stars;
    
    void generate_stars();
    void draw_background();
    void update_stars();
};
