module;

#include "preparser.h"
#include "raylib.h"

export module zeytin.game.crt_effect;
import zeytin.component;

export struct CRTEffect final : public Component
{
    PROPERTY(GROUP="CRT")
    bool enabled = true;

    PROPERTY(GROUP="CRT")
    float curvature = 8.0F;

    PROPERTY(GROUP="CRT")
    float scanline_intensity = 0.08F;

    PROPERTY(GROUP="CRT")
    float vignette_intensity = 0.45F;

    PROPERTY(GROUP="CRT")
    float noise_intensity = 0.02F;

    PROPERTY(GROUP="CRT")
    float chromatic_aberration = 0.25F;

    void on_init() override;
    void on_update() override;

    Shader& get_shader() { return m_shader; }
    bool is_shader_loaded() const { return m_shader_loaded; }

private:
    void load_shader();
    void update_shader_uniforms();
    const char* get_fragment_shader_code() const;

    Shader m_shader{};
    bool m_shader_loaded = false;
    float m_time = 0.0F;
};
