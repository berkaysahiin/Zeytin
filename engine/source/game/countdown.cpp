#include "game/countdown.h"
#include "core/raylib_wrapper.h"
#include "core/zeytin.h"
#include <sstream>
#include <iomanip>

void Countdown::on_init() {
    m_time_remaining = duration;
    m_has_finished = false;
}

void Countdown::on_update() {
    draw_countdown();
}

void Countdown::on_play_start() {
    reset();
}

void Countdown::on_play_update() {
    if (!countdown_active || m_has_finished) {
        return;
    }
    
    m_time_remaining -= get_frame_time();
    
    if (m_time_remaining <= 0.0f) {
        m_time_remaining = 0.0f;
        m_has_finished = true;
        
        if (pause_when_zero) {
        }
    }
}

void Countdown::draw_countdown() {
    float screen_width = VIRTUAL_WIDTH;
    
    std::stringstream time_text;
    
    int minutes = static_cast<int>(m_time_remaining) / 60;
    int seconds = static_cast<int>(m_time_remaining) % 60;
    int centiseconds = static_cast<int>((m_time_remaining - static_cast<int>(m_time_remaining)) * 100);
    
    if (minutes > 0) {
        time_text << std::setfill('0') << std::setw(2) << minutes << ":" 
                  << std::setfill('0') << std::setw(2) << seconds;
    } else {
        time_text << std::setfill('0') << std::setw(2) << seconds;
    }
    
    std::string time_str = time_text.str();
    
    int text_width = MeasureText(time_str.c_str(), font_size);

    float pos_x = screen_width - offset_x - text_width;
    float pos_y = offset_y;
    
    Color current_color = get_current_color();
    
    draw_text(time_str.c_str(), pos_x + 2, pos_y + 2, font_size, {0, 0, 0, 128});
    draw_text(time_str.c_str(), pos_x, pos_y, font_size, current_color);
}

Color Countdown::get_current_color() const {
    if (m_time_remaining <= danger_threshold) {
        return danger_color;
    } else if (m_time_remaining <= warning_threshold) {
        return warning_color;
    } else {
        return normal_color;
    }
}

void Countdown::reset() {
    m_time_remaining = duration;
    m_has_finished = false;
}

void Countdown::add_time(float seconds) {
    m_time_remaining += seconds;
    if (m_time_remaining < 0.0f) {
        m_time_remaining = 0.0f;
    }
    
    if (m_time_remaining > 0.0f) {
        m_has_finished = false;
    }
}
