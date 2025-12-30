#pragma once
#include "variant/variant_base.h"
#include "coverup_utils.h"

class Coverup : public VariantBase {
    VARIANT(Coverup);
    REQUIRES(Position, Scale, Background, CRTEffect);

public:
    int snow_particle_count = 150; PROPERTY();
    float snow_speed_min = 20.0f; PROPERTY();
    float snow_speed_max = 60.0f; PROPERTY();
    float snow_size_min = 2.0f; PROPERTY();
    float snow_size_max = 6.0f; PROPERTY();
    
    int title_font_size = 140; PROPERTY();
    float title_x_offset = 62.20000076293945f; PROPERTY();
    float title_y_position = 352.0f; PROPERTY();
    Color title_color = {255, 50, 50, 255}; PROPERTY();
    Color title_glow_color = {255, 100, 100, 150}; PROPERTY();
    float title_glow_intensity = 0.699999988079071f; PROPERTY();
    
    int zeytin_font_size = 40; PROPERTY();
    float zeytin_x_offset = 62.20000076293945f; PROPERTY();
    float zeytin_y_offset = 460.1000061035156f; PROPERTY();
    Color zeytin_color = {200, 200, 200, 255}; PROPERTY();
    bool show_zeytin_box = true; PROPERTY();
    
    float player_scale = 4.0f; PROPERTY();
    float player_x_position = 386.6000061035156f; PROPERTY();
    float player_y_position = 1000.0f; PROPERTY();
    bool player_face_right = true; PROPERTY();
    float eye_spacing = 0.2f; PROPERTY();
    
    float hat_offset_y = -35.0f; PROPERTY();
    float hat_base_width = 40.0f; PROPERTY();
    float hat_height = 30.0f; PROPERTY();
    float hat_trim_height = 5.0f; PROPERTY();
    float hat_pom_radius = 6.0f; PROPERTY();
    Color hat_color = {200, 0, 0, 255}; PROPERTY();
    Color hat_trim_color = {255, 255, 255, 255}; PROPERTY();
    Color hat_pom_color = {255, 255, 255, 255}; PROPERTY();
    float hat_tilt = 0.15f; PROPERTY();
    
    bool show_enemies = true; PROPERTY();
    int enemy_count = 3; PROPERTY();
    float enemy_scale = 2.5f; PROPERTY();
    
    float enemy1_x = 1234.5999755859375f; PROPERTY();
    float enemy1_y = 823.0999755859375f; PROPERTY();
    bool enemy1_face_right = false; PROPERTY();
    
    float enemy2_x = 482.2999877929687f; PROPERTY();
    float enemy2_y = 480.0f; PROPERTY();
    bool enemy2_face_right = true; PROPERTY();
    
    float enemy3_x = 1607.800048828125f; PROPERTY();
    float enemy3_y = 420.0f; PROPERTY();
    bool enemy3_face_right = false; PROPERTY();
    
    bool show_christmas_lights = true; PROPERTY();
    int light_count = 4; PROPERTY();
    float light_pulse_speed = 5.099999904632568f; PROPERTY();
    float lights_y_position = 113.6999969482422f; PROPERTY();
    float lights_wave_amplitude = 20.0f; PROPERTY();
    
    float screen_center_x = 960.0f; PROPERTY();
    float screen_center_y = 540.0f; PROPERTY();
    
    virtual void on_init() override;
    virtual void on_update() override;

private:
    std::vector<SnowParticle> m_snow_particles;
    std::vector<ChristmasLight> m_lights;
    float m_timer = 0.0f;
    
    void init_snow_particles();
    void init_christmas_lights();
    void update_snow();
    void draw_snow();
    void draw_title();
    void draw_zeytin_credit();
    void draw_player();
    void draw_enemy(float x, float y, float scale, bool face_right);
    void draw_enemies();
    void draw_christmas_lights();
};
