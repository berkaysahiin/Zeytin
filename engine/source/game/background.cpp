#include "game/background.h"
#include "core/raylib_wrapper.h"
#include "core/zeytin.h"
#include <cstdlib>

void Background::on_init() {
    generate_stars();
}

void Background::on_update() {
    update_stars();
    draw_background();
}

void Background::generate_stars() {
    m_stars.clear();
    
    for (int i = 0; i < star_count; i++) {
        Star star;
        star.x = (float)(rand() % (int)VIRTUAL_WIDTH * 1.5f);
        star.y = (float)(rand() % (int)VIRTUAL_HEIGHT);
        star.size = 1.0f + (rand() % 3);
        star.brightness = 0.4f + (rand() % 60) / 100.0f;
        star.speed = star_speed_min + (rand() % (int)(star_speed_max - star_speed_min));
        
        m_stars.push_back(star);
    }
}

void Background::update_stars() {
    for (auto& star : m_stars) {
        star.x -= star.speed * get_frame_time();
        
        // Wrap around when star goes off screen
        if (star.x < 0) {
            star.x = VIRTUAL_WIDTH;
            star.y = (float)(rand() % (int)VIRTUAL_HEIGHT);
        }
    }
}

void Background::draw_background() {
    // Fill background
    clear_background(bg_color);
    
    // Draw stars
    for (const auto& star : m_stars) {
        Color star_color = {
            255,
            255,
            255,
            (unsigned char)(255 * star.brightness)
        };
        
        draw_circle(star.x, star.y, star.size, star_color);
    }
}
