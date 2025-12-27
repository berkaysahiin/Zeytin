#pragma once

#include "variant/variant_base.h"
#include "raylib.h"

class RewindEffect : public VariantBase {
    VARIANT(RewindEffect);
    REQUIRES(TimeController);

public:
    bool enabled = true; PROPERTY();
    
    //Color rewind_tint = {100, 100, 255, 40}; PROPERTY();  
    int arrow_count = 3; PROPERTY();                       
    float arrow_size = 60.0f; PROPERTY();                 
    float arrow_spacing = 30.0f; PROPERTY();               
    Color arrow_color = {255, 255, 255, 200}; PROPERTY();
    Color arrow_bg_color = {0, 0, 0, 100}; PROPERTY();     
    
    float pulse_speed = 2.0f; PROPERTY();                  
    float pulse_intensity = 0.2f; PROPERTY();             
    
    virtual void on_init() override;
    virtual void on_update() override;

private:
    void draw_rewind_overlay();
    void draw_center_panel();
    void draw_arrow(float x, float y, float scale);
    
    float m_timer = 0.0f;
};
