#include "game/coverup.h"
#include "core/raylib_wrapper.h"
#include <cmath>
#include <cstdlib>

void Coverup::on_init() {
    m_timer = 0.0f;
    init_snow_particles();
    init_christmas_lights();
}

void Coverup::on_update() {
    m_timer += get_frame_time();
    
    update_snow();
    
    draw_snow();
    draw_christmas_lights();
    draw_enemies();
    draw_player();
    draw_title();
    draw_zeytin_credit();
}

void Coverup::init_snow_particles() {
    m_snow_particles.clear();
    m_snow_particles.reserve(snow_particle_count);
    
    int screen_width = get_screen_width();
    int screen_height = get_screen_height();
    
    for (int i = 0; i < snow_particle_count; i++) {
        SnowParticle particle;
        particle.x = static_cast<float>(rand() % screen_width);
        particle.y = static_cast<float>(rand() % screen_height);
        particle.speed = snow_speed_min + static_cast<float>(rand()) / RAND_MAX * (snow_speed_max - snow_speed_min);
        particle.size = snow_size_min + static_cast<float>(rand()) / RAND_MAX * (snow_size_max - snow_size_min);
        m_snow_particles.push_back(particle);
    }
}

void Coverup::init_christmas_lights() {
    m_lights.clear();
    m_lights.reserve(light_count);
    
    Color light_colors[] = {
        {255, 0, 0, 255},    // Red
        {0, 255, 0, 255},    // Green
        {255, 255, 0, 255},  // Yellow
        {0, 0, 255, 255},    // Blue
        {255, 255, 255, 255} // White
    };
    
    float screen_width = static_cast<float>(get_screen_width());
    float spacing = screen_width / (light_count - 1);
    
    for (int i = 0; i < light_count; i++) {
        ChristmasLight light;
        light.x = spacing * i;
        light.y = lights_y_position + sin(i * 0.5f) * lights_wave_amplitude;
        light.color = light_colors[i % 5];
        light.phase = static_cast<float>(i) * 0.5f;
        m_lights.push_back(light);
    }
}

void Coverup::update_snow() {
    float dt = get_frame_time();
    int screen_width = get_screen_width();
    int screen_height = get_screen_height();
    
    for (auto& particle : m_snow_particles) {
        particle.y += particle.speed * dt;
        particle.x += sin(m_timer + particle.y * 0.01f) * 20.0f * dt;
        
        if (particle.y > screen_height) {
            particle.y = -10.0f;
            particle.x = static_cast<float>(rand() % screen_width);
        }
        
        if (particle.x < -10.0f) {
            particle.x = static_cast<float>(screen_width + 10);
        } else if (particle.x > screen_width + 10) {
            particle.x = -10.0f;
        }
    }
}

void Coverup::draw_snow() {
    for (const auto& particle : m_snow_particles) {
        draw_circle(particle.x, particle.y, particle.size, WHITE);
        draw_circle(particle.x, particle.y, particle.size * 0.6f, 
                   ColorAlpha(WHITE, 0.5f));
    }
}

void Coverup::draw_title() {
    const char* title = "DEFUSE";
    
    int screen_width = get_screen_width();
    int text_width = MeasureText(title, title_font_size);
    
    float center_x = (screen_width / 2.0f) + title_x_offset - (text_width / 2.0f);
    
    for (int i = 0; i < 3; i++) {
        float offset = (3 - i) * 4.0f;
        float alpha = (3 - i) * 0.3f * title_glow_intensity;
        Color glow = title_glow_color;
        glow.a = static_cast<unsigned char>(glow.a * alpha);
        
        draw_text(title, 
                 center_x - offset, 
                 title_y_position - offset, 
                 title_font_size, 
                 glow);
    }
    
    draw_text(title, 
             center_x, 
             title_y_position, 
             title_font_size, 
             title_color);
    
    float pulse = 0.5f + 0.5f * sin(m_timer * 2.0f);
    draw_text(title, 
             center_x + 4, 
             title_y_position + 4, 
             title_font_size, 
             ColorAlpha(BLACK, pulse * 0.6f));
}

void Coverup::draw_zeytin_credit() {
    const char* credit = "Made with Zeytin";
    
    int screen_width = get_screen_width();
    int screen_height = get_screen_height();
    int text_width = MeasureText(credit, zeytin_font_size);
    
    float center_x = (screen_width / 2.0f) + zeytin_x_offset - (text_width / 2.0f);
    float y_pos = screen_height - zeytin_y_offset;
    
    if (show_zeytin_box) {
        float padding = 15.0f;
        draw_rectangle(
            center_x - padding,
            y_pos - padding,
            text_width + padding * 2,
            zeytin_font_size + padding * 2,
            ColorAlpha(BLACK, 0.6f)
        );
        
        draw_rectangle_lines(
            center_x - padding,
            y_pos - padding,
            text_width + padding * 2,
            zeytin_font_size + padding * 2,
            ColorAlpha(zeytin_color, 0.8f)
        );
    }
    
    draw_text(credit, 
             center_x, 
             y_pos, 
             zeytin_font_size, 
             zeytin_color);
}

void Coverup::draw_player() {
    float x = player_x_position;
    float y = player_y_position;
    float body_size = 50.0f * player_scale;
    
    Color body_color = {88, 83, 86, 255};
    draw_rectangle(
        x - body_size / 2,
        y - body_size / 2,
        body_size,
        body_size,
        body_color
    );
    
    float eye_size = 8.0f * player_scale;
    float eye_offset_x = 12.0f * player_scale * (player_face_right ? 1.0f : -1.0f);
    float eye_offset_y = -8.0f * player_scale;
    float actual_eye_spacing = eye_spacing * body_size;
    
    draw_circle(x - actual_eye_spacing / 2 + eye_offset_x, y + eye_offset_y, eye_size, WHITE);
    draw_circle(x + actual_eye_spacing / 2 + eye_offset_x, y + eye_offset_y, eye_size, WHITE);
    
    float scaled_hat_offset_y = hat_offset_y * player_scale;
    float scaled_hat_base_width = hat_base_width * player_scale;
    float scaled_hat_height = hat_height * player_scale;
    float scaled_hat_trim_height = hat_trim_height * player_scale;
    float scaled_hat_pom_radius = hat_pom_radius * player_scale;
    
    float hat_base_y = y - body_size / 2 + scaled_hat_offset_y;
    float hat_tip_y = hat_base_y - scaled_hat_height;
    float hat_left_x = x - scaled_hat_base_width / 2;
    float hat_right_x = x + scaled_hat_base_width / 2;
    float hat_tip_x = x + (scaled_hat_base_width * hat_tilt);
    
    draw_triangle(
        {hat_left_x, hat_base_y},
        {hat_right_x, hat_base_y},
        {hat_tip_x, hat_tip_y},
        hat_color
    );
    
    draw_rectangle(
        hat_left_x,
        hat_base_y,
        scaled_hat_base_width,
        scaled_hat_trim_height,
        hat_trim_color
    );
    
    draw_circle(hat_tip_x, hat_tip_y, scaled_hat_pom_radius, hat_pom_color);
}

void Coverup::draw_enemy(float x, float y, float scale, bool face_right) {
    float body_size = 40.0f * scale;
    
    Color body_color = {180, 50, 50, 255};
    draw_rectangle(
        x - body_size / 2,
        y - body_size / 2,
        body_size,
        body_size,
        body_color
    );
    
    draw_rectangle_lines(
        x - body_size / 2 - 2,
        y - body_size / 2 - 2,
        body_size + 4,
        body_size + 4,
        ColorAlpha({255, 100, 100, 255}, 0.5f)
    );
    
    float eye_size = 6.0f * scale;
    float eye_offset_x = 10.0f * scale * (face_right ? 1.0f : -1.0f);
    float eye_offset_y = -6.0f * scale;
    float actual_eye_spacing = eye_spacing * body_size;
    
    Color eye_color = {255, 0, 0, 255};
    draw_circle(x - actual_eye_spacing / 2 + eye_offset_x, y + eye_offset_y, eye_size, eye_color);
    draw_circle(x + actual_eye_spacing / 2 + eye_offset_x, y + eye_offset_y, eye_size, eye_color);
    
    draw_circle(x - actual_eye_spacing / 2 + eye_offset_x, y + eye_offset_y, eye_size * 1.5f, 
               ColorAlpha(eye_color, 0.3f));
    draw_circle(x + actual_eye_spacing / 2 + eye_offset_x, y + eye_offset_y, eye_size * 1.5f, 
               ColorAlpha(eye_color, 0.3f));
    
    float horn_height = 8.0f * scale;
    float horn_width = 3.0f * scale;
    float horn_spacing = 15.0f * scale;
    
    draw_rectangle(
        x - horn_spacing - horn_width / 2,
        y - body_size / 2 - horn_height,
        horn_width,
        horn_height,
        body_color
    );
    
    draw_rectangle(
        x + horn_spacing - horn_width / 2,
        y - body_size / 2 - horn_height,
        horn_width,
        horn_height,
        body_color
    );
    
    draw_circle(x - horn_spacing, y - body_size / 2 - horn_height, horn_width, 
               ColorAlpha({255, 0, 0, 255}, 0.8f));
    draw_circle(x + horn_spacing, y - body_size / 2 - horn_height, horn_width, 
               ColorAlpha({255, 0, 0, 255}, 0.8f));
}

void Coverup::draw_enemies() {
    if (!show_enemies) return;
    
    // Draw up to enemy_count enemies
    if (enemy_count >= 1) {
        draw_enemy(enemy1_x, enemy1_y, enemy_scale, enemy1_face_right);
    }
    
    if (enemy_count >= 2) {
        draw_enemy(enemy2_x, enemy2_y, enemy_scale, enemy2_face_right);
    }
    
    if (enemy_count >= 3) {
        draw_enemy(enemy3_x, enemy3_y, enemy_scale, enemy3_face_right);
    }
}

void Coverup::draw_christmas_lights() {
    if (!show_christmas_lights) return;
    
    for (size_t i = 0; i < m_lights.size() - 1; i++) {
        draw_line(
            m_lights[i].x, m_lights[i].y,
            m_lights[i + 1].x, m_lights[i + 1].y,
            ColorAlpha(GRAY, 0.5f)
        );
    }
    
    for (const auto& light : m_lights) {
        float pulse = 0.7f + 0.3f * sin(m_timer * light_pulse_speed + light.phase);
        
        draw_circle(light.x, light.y, 12.0f, 
                   ColorAlpha(light.color, pulse * 0.3f));
        
        draw_circle(light.x, light.y, 8.0f, 
                   ColorAlpha(light.color, pulse));
        
        draw_circle(light.x - 2, light.y - 2, 3.0f, 
                   ColorAlpha(WHITE, pulse * 0.8f));
    }
}
