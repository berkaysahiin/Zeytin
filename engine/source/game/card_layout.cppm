module;

#include <cstdint>
#include <type_traits>
#include <vector>

export module zeytin.game.card_layout;
import zeytin.query;
import zeytin.entity;
import zeytin.zeytin;
import zeytin.logger;

import zeytin.game.aliases;
import zeytin.game.card;
import zeytin.game.card_board_config;
import zeytin.game.card_config;
import zeytin.game.transform;

export inline void clear_card_layout() {
    const std::vector<EntityID> ids = Query::find_all_with<CCard>();
    for (const EntityID entity_id : ids) {
        Zeytin::get().remove_entity(entity_id);
    }
}

export template<typename... Ts>
void setup_card_layout(const GCardBoardConfig& board, const GCardConfig& config,
					   [[maybe_unused]] const Alias<Ts...>& alias) {
    static_assert(alias_contains_v<CCard, Ts...>, "Alias must include CCard");
    static_assert(alias_contains_v<CTransform, Ts...>, "Alias must include CTransform");

    const int32_t rows = board.rows > 0 ? board.rows : 1;
    const int32_t columns = board.columns > 0 ? board.columns : 1;

    float canvas_width = board.canvas_width > 1.0F ? board.canvas_width : 1.0F;
    float canvas_height = board.canvas_height > 1.0F ? board.canvas_height : 1.0F;

    float origin_x = board.canvas_x;
    float origin_y = board.canvas_y;

    if (board.canvas_centered) {
        origin_x -= canvas_width * 0.5F;
        origin_y -= canvas_height * 0.5F;
    }

    const bool use_virtual = board.use_virtual_canvas;
    if (use_virtual) {
        canvas_width = VIRTUAL_WIDTH;
        canvas_height = VIRTUAL_HEIGHT;
    }

    const float cell_width = canvas_width / static_cast<float>(columns);
    const float cell_height = canvas_height / static_cast<float>(rows);

    const float card_width = config.card_width < cell_width ? config.card_width : cell_width;
    const float card_height = config.card_height < cell_height ? config.card_height : cell_height;

    const float offset_x = (cell_width - card_width) * 0.5F;
    const float offset_y = (cell_height - card_height) * 0.5F;

    log_info(
        "Card layout: rows={} cols={} canvas=({:.2f},{:.2f},{:.2f},{:.2f}) cell=({:.2f},{:.2f}) card=({:.2f},{:.2f}) origin=({:.2f},{:.2f}) virtual={}",
        rows,
        columns,
        board.canvas_x,
        board.canvas_y,
        canvas_width,
        canvas_height,
        cell_width,
        cell_height,
        card_width,
        card_height,
        origin_x,
        origin_y,
        use_virtual
    );

    for (int32_t row = 0; row < rows; ++row) {
        for (int32_t col = 0; col < columns; ++col) {
            const EntityID entity_id = Query::create_entity();

            (Query::add<Ts>(entity_id), ...);

            auto& card = Query::get<CCard>(entity_id);
            card.row = row;
            card.column = col;
            card.is_face_up = false;
            card.is_matched = false;

            auto& transform = Query::get<CTransform>(entity_id);
            transform.position_x = origin_x + (cell_width * static_cast<float>(col)) + offset_x;
            transform.position_y = origin_y + (cell_height * static_cast<float>(row)) + offset_y;
            transform.rotation = 0.0F;
            transform.scale_x = 1.0F;
            transform.scale_y = 1.0F;
        }
    }
}
