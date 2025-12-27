#pragma once

#include "variant/variant_base.h"
#include "raylib.h"

class CRTEffect : public VariantBase {
    VARIANT(CRTEffect);

public:
    bool enabled = true; PROPERTY();
    
    float curvature = 4.0f; PROPERTY();
    float scanline_intensity = 0.3f; PROPERTY();
    float vignette_intensity = 0.3f; PROPERTY();
    float noise_intensity = 0.05f; PROPERTY();
    float chromatic_aberration = 0.5f; PROPERTY();
    
    virtual void on_init() override;
    virtual void on_update() override;

    Shader& get_shader() { return m_shader; }
    bool is_shader_loaded() const { return m_shader_loaded; }

private:
    void load_shader();
    void update_shader_uniforms();
    const char* get_fragment_shader_code();
    
    Shader m_shader;
    bool m_shader_loaded = false;
    float m_time = 0.0f;
};
