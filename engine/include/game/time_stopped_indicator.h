#pragma once

#include "variant/variant_base.h"
#include "raylib.h"

class TimeStoppedIndicator : public VariantBase {
    VARIANT(TimeStoppedIndicator);
public:
    bool enabled = true; PROPERTY();
    int font_size = 60; PROPERTY();
    float padding = 20.0f; PROPERTY();
	float activate_threshhold = 0.3f; PROPERTY()
    
    Color text_color = {255, 255, 255, 200}; 
    Color bg_color = {0, 0, 0, 150}; 

    virtual void on_update() override;

private:
    void draw_indicator();
	float m_stoppedDuration;
};
