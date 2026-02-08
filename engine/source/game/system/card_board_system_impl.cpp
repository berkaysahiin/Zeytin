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

    GCardBoardConfig board = board_opt->get();
    int32_t total_cards = board.rows * board.columns;
    if (total_cards % 2 != 0) {
        log_error("Odd card count ({}). Adjusting layout to even.", total_cards);
        if (board.columns > 1) {
            board.columns -= 1;
        } else if (board.rows > 1) {
            board.rows -= 1;
        }
        total_cards = board.rows * board.columns;
    }
    log_info(
        "Setting up card layout with rows={} cols={} canvas_width={:.2f} canvas_height={:.2f} use_virtual_canvas={}",
        board.rows,
        board.columns,
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

    const int32_t rows = board.rows;
    const int32_t columns = board.columns;

    const int32_t symbol_matrix_4x4[4][4] = {
        {0, 1, 2, 3},
        {4, 5, 6, 7},
        {2, 3, 0, 1},
        {6, 7, 4, 5}
    };

    const int32_t mask_matrix_4x4[4][4] = {
        {1, 1, 2, 2},
        {1, 2, 1, 2},
        {2, 1, 2, 1},
        {2, 2, 1, 1}
    };

    const int32_t symbol_matrix_2x2[2][2] = {
        {0, 1},
        {1, 0}
    };

    const int32_t mask_matrix_2x2[2][2] = {
        {1, 2},
        {2, 1}
    };

    const int32_t symbol_matrix_4x3[4][3] = {
        {5, 1, 5},
        {3, 0, 1},
        {2, 4, 3},
        {4, 2, 0}
    };

    const int32_t mask_matrix_4x3[4][3] = {
        {1, 2, 1},
        {2, 1, 2},
        {1, 2, 1},
        {2, 1, 2}
    };

    const std::vector<EntityID> card_ids = Query::find_all_with<CCard>();
    for (const EntityID id : card_ids) {
        auto& card = Query::get<CCard>(id);
        const int row = card.row;
        const int col = card.column;
        if (rows == 2 && columns == 2 && row < 2 && col < 2) {
            card.symbol_id = symbol_matrix_2x2[row][col];
        } else if (rows == 4 && columns == 3 && row < 4 && col < 3) {
            card.symbol_id = symbol_matrix_4x3[row][col];
        } else if (row < 4 && col < 4) {
            card.symbol_id = symbol_matrix_4x4[row][col];
        } else {
            card.symbol_id = 0;
        }
        card.e_mask_status = CardMaskStatus::NO_MASK;
    }

    std::array<int, 16> symbol_counts{};
    bool has_odd = false;
    for (const EntityID id : card_ids) {
        const auto& card = Query::get<CCard>(id);
        if (card.symbol_id >= 0 && card.symbol_id < static_cast<int>(symbol_counts.size())) {
            symbol_counts[card.symbol_id] += 1;
        }
    }

    for (size_t i = 0; i < symbol_counts.size(); ++i) {
        if (symbol_counts[i] % 2 != 0) {
            has_odd = true;
            log_error("Symbol {} has odd count {} (unsolvable layout)", i, symbol_counts[i]);
        }
    }

    if (has_odd) {
        const int total_cards = rows * columns;
        const int pair_count = total_cards / 2;
        std::vector<int> pairs;
        pairs.reserve(static_cast<size_t>(total_cards));
        for (int i = 0; i < pair_count; ++i) {
            pairs.push_back(i);
            pairs.push_back(i);
        }

        int index = 0;
        for (const EntityID id : card_ids) {
            if (index >= total_cards) {
                break;
            }
            auto& card = Query::get<CCard>(id);
            card.symbol_id = pairs[static_cast<size_t>(index)];
            index++;
        }
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
		const auto board_opt = Query::try_find_first<GCardBoardConfig>();
		const GCardBoardConfig board = board_opt ? board_opt->get() : GCardBoardConfig{};
		const int32_t rows = board.rows;
		const int32_t columns = board.columns;

		const int32_t mask_matrix_4x4[4][4] = {
			{1, 1, 2, 2},
			{1, 2, 1, 2},
			{2, 1, 2, 1},
			{2, 2, 1, 1}
		};

		const int32_t mask_matrix_2x2[2][2] = {
			{1, 2},
			{2, 1}
		};

		const int32_t mask_matrix_4x3[4][3] = {
			{1, 2, 1},
			{2, 1, 2},
			{1, 2, 1},
			{2, 1, 2}
		};

		const std::vector<EntityID> card_ids = Query::find_all_with<CCard>();
		for (const EntityID id : card_ids) {
			auto& card = Query::get<CCard>(id);
			const int row = card.row;
			const int col = card.column;
		if (rows == 2 && columns == 2 && row < 2 && col < 2) {
			const int mask_val = mask_matrix_2x2[row][col];
			card.e_mask_status = static_cast<CardMaskStatus>(mask_val);
		} else if (rows == 4 && columns == 3 && row < 4 && col < 3) {
			const int mask_val = mask_matrix_4x3[row][col];
			card.e_mask_status = static_cast<CardMaskStatus>(mask_val);
		} else if (row < 4 && col < 4) {
			const int mask_val = mask_matrix_4x4[row][col];
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
