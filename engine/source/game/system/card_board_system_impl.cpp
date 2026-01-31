module;
#include <optional>
#include <functional>
#include "raylib.h"

module zeytin.game.card_board_system;
import zeytin.query;
import zeytin.logger;
import zeytin.game.card_layout;
import zeytin.game.card_board_config;
import zeytin.game.card_config;
import zeytin.game.transform;
import zeytin.game.aliases;
import zeytin.game.card;
import zeytin.raylib;
import zeytin.entity;

namespace {
    template<typename T>
    std::optional<std::reference_wrapper<T>> find_config_stable() {
        const auto configs = Query::find_all<T>();
        if (configs.empty()) {
            return std::nullopt;
        }

        auto best = configs.front();
        for (const auto& config : configs) {
            if (config.get().get_id() < best.get().get_id()) {
                best = config;
            }
        }

        return best;
    }
}

void CCardBoardSystem::on_play_start() {
    const auto board_opt = find_config_stable<GCardBoardConfig>();
    const auto card_opt = find_config_stable<GCardConfig>();
    if (!board_opt || !card_opt) {
        if (!board_opt) {
            log_error("Card board config missing; cannot layout cards");
        }
        if (!card_opt) {
            log_error("Card config missing; cannot layout cards");
        }
        return;
    }

    clear_card_layout();
    setup_card_layout(
        board_opt->get(),
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
        {4, 5, 0, 1},
        {2, 3, 4, 5},
        {0, 1, 2, 3}
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

void CCardBoardSystem::on_update() {
    const auto card_opt = find_config_stable<GCardConfig>();
    if (!card_opt) {
        return;
    }

    const GCardConfig& config = card_opt->get();

    if (m_revealed) {
        return;
    }

    m_reveal_timer += get_frame_time();

    if (m_reveal_timer >= config.initial_reveal_duration) {
        const std::vector<EntityID> card_ids = Query::find_all_with<CCard>();
        for (const EntityID id : card_ids) {
            auto& card = Query::get<CCard>(id);
            const CardMaskStatus mask_type = static_cast<CardMaskStatus>(config.mask_type);
            card.e_mask_status = mask_type;
        }
        m_revealed = true;
        log_info("All cards hidden after {:.2f} seconds", config.initial_reveal_duration);
    }
}
