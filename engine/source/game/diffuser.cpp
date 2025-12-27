#include "game/diffuser.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/position.h"
#include "game/scale.h"
#include "game/player.h"
#include "game/game_manager.h"

void Diffuser::on_init() {
    m_picked_up = false;
    m_bob_timer = 0.0f;
}

void Diffuser::on_update() {
    if (!m_picked_up) {
        draw_diffuser();
        draw_pickup_circle();
    }
}

void Diffuser::on_play_update() {
    if (m_picked_up) return;
    
    // Only run if game is running
    auto game_mgr_opt = Query::try_find_first<GameManager>();
    if (!game_mgr_opt || !game_mgr_opt->get().should_game_run()) {
        return;
    }
    
    check_player_pickup();
}

void Diffuser::check_player_pickup() {
    auto& diffuser_pos = Query::get<Position>(this);
    auto player_opt = Query::try_find_first<Player>();
    
    if (!player_opt) return;
    
    auto& player_pos = Query::get<Position>(&player_opt->get());
    
    float dx = player_pos.x - diffuser_pos.x;
    float dy = player_pos.y - diffuser_pos.y;
    float distance = sqrt(dx * dx + dy * dy);
    
    if (distance <= pickup_radius) {
        m_picked_up = true;
        
        // Notify player they picked it up
        auto& player = player_opt->get();
        player.set_has_diffuser(true);
    }
}

void Diffuser::draw_diffuser() {
    auto& position = Query::get<Position>(this);
    auto& scale = Query::get<Scale>(this);
    
    // Update bob animation
    m_bob_timer += get_frame_time() * bob_speed;
    float bob_offset = sin(m_bob_timer) * bob_amount;
    
    float scaled_size = icon_size * scale.x;
    float draw_y = position.y + bob_offset;
    
    // Draw diffuser tool (wrench/tool icon shape)
    // Main body
    draw_rectangle(
        position.x - scaled_size / 4,
        draw_y - scaled_size / 2,
        scaled_size / 2,
        scaled_size,
        diffuser_color
    );
    
    // Handle
    draw_rectangle(
        position.x - scaled_size / 6,
        draw_y + scaled_size / 4,
        scaled_size / 3,
        scaled_size / 2,
        diffuser_color
    );
    
    // Glow effect
    draw_circle(position.x, draw_y, scaled_size * 0.8f, 
               ColorAlpha(diffuser_color, 0.3f));
    
    // Label
    const char* label = "DIFFUSER";
    int font_size = 20;
    int text_width = MeasureText(label, font_size);
    draw_text(label,
            position.x - text_width / 2,
            draw_y + scaled_size + 5,
            font_size,
            diffuser_color);
}

void Diffuser::draw_pickup_circle() {
    auto& position = Query::get<Position>(this);
    auto& scale = Query::get<Scale>(this);
    
    float scaled_radius = pickup_radius * scale.x;
    
    // Pulsing effect
    float pulse = 0.5f + 0.5f * sin(m_bob_timer * 2.0f);
    Color circle_col = ColorAlpha(pickup_circle_color, pulse * 0.5f);
    
    draw_circle_lines(position.x, position.y, scaled_radius, circle_col);
    draw_circle(position.x, position.y, scaled_radius, 
               ColorAlpha(circle_col, 0.1f));
}
