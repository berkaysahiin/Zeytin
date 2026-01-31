module;
#include <optional>
#include <functional>
#include <string>
#include "raylib.h"

module zeytin.game.card_board_system;
import zeytin.query;
import zeytin.logger;
import zeytin.game.card_layout;
import zeytin.game.card_board_config;
import zeytin.game.card_config;
import zeytin.game.ui_config;
import zeytin.game.transform;
import zeytin.game.aliases;
import zeytin.game.card;
import zeytin.raylib;
import zeytin.entity;

void CCardBoardSystem::on_play_start() {
    const auto board_opt = Query::try_find_first<GCardBoardConfig>();
    const auto card_opt = Query::try_find_first<GCardConfig>();
    if (!board_opt || !card_opt) {
        if (!board_opt) {
            log_error("Card board config missing; cannot layout cards");
        }
        if (!card_opt) {
            log_error("Card config missing; cannot layout cards");
        }
        return;
    }

    const GCardBoardConfig& board = board_opt->get();
    log_info(
        "Setting up card layout with canvas_width={:.2f} canvas_height={:.2f} use_virtual_canvas={}",
        board.canvas_width,
        board.canvas_height,
        board.use_virtual_canvas
    );

    clear_card_layout();
    setup_card_layout(
        board,
        card_opt->get(),
        this->initial_collider_width,
        this->initial_collider_height,
        CardAlias{}
    );

    m_reveal_timer = 0.0F;
    m_revealed = false;

    const int32_t rows = board_opt->get().rows;
    const int32_t columns = board_opt->get().columns;

    const int32_t symbol_matrix[4][4] = {
        {0, 1, 2, 3},
        {4, 0, 1, 2},
        {3, 4, 0, 1},
        {2, 3, 4, 0}
    };

    const int32_t mask_matrix[4][4] = {
        {1, 1, 2, 2},
        {1, 2, 1, 2},
        {2, 1, 2, 1},
        {2, 2, 1, 1}
    };

    const std::vector<EntityID> card_ids = Query::find_all_with<CCard>();
    for (const EntityID id : card_ids) {
        auto& card = Query::get<CCard>(id);
        const int row = card.row;
        const int col = card.column;
        if (row < 4 && col < 4) {
            card.symbol_id = symbol_matrix[row][col];
        } else {
            card.symbol_id = 0;
        }
        card.e_mask_status = CardMaskStatus::NO_MASK;
    }

    log_info("Assigned symbols from matrix to {} cards", card_ids.size());
}

void CCardBoardSystem::on_play_update() {
    const auto card_opt = Query::try_find_first<GCardConfig>();
    if (!card_opt) {
        return;
    }

    const GCardConfig& config = card_opt->get();

    if (m_revealed) {
        return;
    }

    m_reveal_timer += get_frame_time();

	if (m_reveal_timer >= (config.initial_reveal_duration + config.reveal_hold_after_timer)) {
        const int32_t mask_matrix[4][4] = {
            {1, 1, 2, 2},
            {1, 2, 1, 2},
            {2, 1, 2, 1},
            {2, 2, 1, 1}
        };

        const std::vector<EntityID> card_ids = Query::find_all_with<CCard>();
        for (const EntityID id : card_ids) {
            auto& card = Query::get<CCard>(id);
            const int row = card.row;
            const int col = card.column;
            if (row < 4 && col < 4) {
                const int mask_val = mask_matrix[row][col];
                card.e_mask_status = static_cast<CardMaskStatus>(mask_val);
            } else {
                card.e_mask_status = static_cast<CardMaskStatus>(config.mask_type);
            }
        }
        m_revealed = true;
        log_info("Hiding {} cards with masks after {:.2f} seconds", card_ids.size(), config.initial_reveal_duration);
    }
}

void CCardBoardSystem::on_update() {
    const auto card_opt = Query::try_find_first<GCardConfig>();
    if (!card_opt) {
        return;
    }

    const GCardConfig& config = card_opt->get();

    if (m_revealed) {
        return;
    }

	const float remaining_time = (m_reveal_timer >= config.initial_reveal_duration) ? 0.0F : (config.initial_reveal_duration - m_reveal_timer);

    const auto ui_opt = Query::try_find_first<GGameUIConfig>();
    const int font_size = ui_opt ? ui_opt->get().timer_font_size : 32;
    const bool show_go = remaining_time <= 0.0F && m_reveal_timer < (config.initial_reveal_duration + config.reveal_hold_after_timer);
    const int time_int = static_cast<int>(remaining_time);
    const std::string text = show_go ? "GO" : ("Time: " + std::to_string(time_int));
    if (!show_go && remaining_time <= 0.0F) {
        return;
    }

    const float text_x = ui_opt ? ui_opt->get().timer_x : 20.0F;
    const float text_y = ui_opt ? ui_opt->get().timer_y : 20.0F;
    const Color text_color = show_go ? Color{.r=255, .g=220, .b=120, .a=255} : Color{.r=200, .g=200, .b=200, .a=255};

    DrawText(text.c_str(), static_cast<int>(text_x), static_cast<int>(text_y), font_size, text_color);
}
