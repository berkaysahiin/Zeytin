module;
#include "raylib.h"
#include <vector>
#include <set>
#include <string>

module zeytin.game.matching_game;
import zeytin.query;
import zeytin.logger;
import zeytin.game.card;
import zeytin.game.transform;
import zeytin.game.collider;
import zeytin.raylib;
import zeytin.zeytin;
import zeytin.entity;

namespace {
	Color make_color(const int red, const int green, const int blue, const int alpha) {
		return Color{
			.r=static_cast<unsigned char>(red),
			.g=static_cast<unsigned char>(green),
			.b=static_cast<unsigned char>(blue),
			.a=static_cast<unsigned char>(alpha)
		};
	}
}

void GMatchingGame::on_play_start() {
	m_first_selected = 0;
	m_second_selected = 0;
	m_remaining_actions = allowed_actions;
	m_found_pairs.clear();
	m_game_over = false;
	log_info("Matching game started with {} actions", m_remaining_actions);
}

void GMatchingGame::on_play_update() {
	if (m_game_over) {
		draw_game_over_overlay();
		return;
	}

	draw_actions_ui();
	draw_selection_highlight();

	if (handle_mouse_click()) {
		check_game_state();
	}
}

bool GMatchingGame::handle_mouse_click() {
	if (!is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
		return false;
	}

	const Vector2 mouse_pos = get_mouse_position();
	const Camera2D& camera = Zeytin::get().get_camera();
	const Vector2 world_mouse = get_screen_to_world2d(mouse_pos, camera);

	const auto card_ids = Query::find_all_with<CCard, CCollider>();
	for (const EntityID id : card_ids) {
		auto& card = Query::get<CCard>(id);
		auto& collider = Query::get<CCollider>(id);

		if (collider.is_point_inside(world_mouse.x, world_mouse.y)) {
			if (m_first_selected == id) {
				return false;
			}

			process_card_selection(id, card);
			return true;
		}
	}
	return false;
}

void GMatchingGame::process_card_selection(EntityID id, CCard& card) {
	if (m_first_selected == 0) {
		if (card.e_mask_status == CardMaskStatus::NO_MASK) {
			return;
		}

		m_first_selected = id;
		m_first_mask_type = card.e_mask_status;
		card.e_mask_status = CardMaskStatus::NO_MASK;
		log_info("First selected: card {}, mask type {}", id, static_cast<int>(m_first_mask_type));
	} else {
		m_second_selected = id;
		m_second_mask_type = card.e_mask_status;

		m_remaining_actions--;

		auto& first_card = Query::get<CCard>(m_first_selected);

		if (card.e_mask_status != CardMaskStatus::NO_MASK) {
			card.e_mask_status = CardMaskStatus::NO_MASK;

			if (first_card.symbol_id == card.symbol_id) {
				m_found_pairs.insert(first_card.symbol_id);
				log_info("Match found: symbol {}", first_card.symbol_id);
			} else {
				first_card.e_mask_status = m_first_mask_type;
				card.e_mask_status = m_second_mask_type;
				log_info("No match: symbols {} vs {}", first_card.symbol_id, card.symbol_id);
			}
		} else {
			card.e_mask_status = m_first_mask_type;
			log_info("Move: mask from card {} to {}", m_first_selected, id);
		}

		m_first_selected = 0;
		m_second_selected = 0;
	}
}

void GMatchingGame::check_game_state() {
	const int total_unique_symbols = 6;
	const bool all_found = m_found_pairs.size() == total_unique_symbols;

	if (all_found) {
		m_game_over = true;
		log_info("YOU WIN! All pairs found in {} actions", allowed_actions - m_remaining_actions);
	} else if (m_remaining_actions <= 0) {
		m_game_over = true;
		log_info("GAME OVER. Found {}/{} pairs", m_found_pairs.size(), total_unique_symbols);
	}
}

void GMatchingGame::draw_selection_highlight() {
	if (m_first_selected == 0) {
		return;
	}

	const auto& transform = Query::get<CTransform>(m_first_selected);
	const auto& collider = Query::get<CCollider>(m_first_selected);

	const float outline_thickness = 4.0F;
	const Rectangle rect{
		.x=transform.position_x - collider.width / 2.0F - outline_thickness,
		.y=transform.position_y - collider.height / 2.0F - outline_thickness,
		.width=collider.width + outline_thickness * 2.0F,
		.height=collider.height + outline_thickness * 2.0F
	};

	const Color highlight_color = make_color(255, 215, 0, 255);
	DrawRectangleLinesEx(rect, outline_thickness, highlight_color);
}

void GMatchingGame::draw_actions_ui() {
	const int font_size = 24;
	const std::string text = "Actions: " + std::to_string(m_remaining_actions);
	const int text_width = MeasureText(text.c_str(), font_size);

	const float margin = 20.0F;
	const float screen_width = static_cast<float>(get_screen_width());
	const float x = screen_width - text_width - margin;
	const float y = margin;

	const Color text_color = make_color(200, 200, 200, 255);
	DrawText(text.c_str(), static_cast<int>(x), static_cast<int>(y), font_size, text_color);
}

void GMatchingGame::draw_game_over_overlay() {
	const int font_size = 48;
	const float screen_width = static_cast<float>(get_screen_width());
	const float screen_height = static_cast<float>(get_screen_height());

	const int total_unique_symbols = 6;
	const bool all_found = m_found_pairs.size() == total_unique_symbols;

	const std::string message = all_found ? "YOU WIN!" : "GAME OVER";
	const Color text_color = all_found ? make_color(100, 200, 100, 255) : make_color(200, 100, 100, 255);

	const int text_width = MeasureText(message.c_str(), font_size);
	const float x = (screen_width - text_width) * 0.5F;
	const float y = screen_height * 0.5F - font_size * 2.0F;

	const Color bg_color = make_color(30, 30, 30, 200);
	DrawRectangle(0, 0, screen_width, screen_height, bg_color);

	DrawText(message.c_str(), static_cast<int>(x), static_cast<int>(y), font_size, text_color);

	const int sub_font_size = 24;
	const std::string pairs_text = "Pairs: " + std::to_string(m_found_pairs.size()) + "/" + std::to_string(total_unique_symbols);
	const int pairs_width = MeasureText(pairs_text.c_str(), sub_font_size);
	const float pairs_x = (screen_width - pairs_width) * 0.5F;
	const float pairs_y = y + font_size + 20.0F;

	const Color sub_color = make_color(150, 150, 150, 255);
	DrawText(pairs_text.c_str(), static_cast<int>(pairs_x), static_cast<int>(pairs_y), sub_font_size, sub_color);

	const int restart_font_size = 20;
	const std::string restart_text = "Press R to restart";
	const int restart_width = MeasureText(restart_text.c_str(), restart_font_size);
	const float restart_x = (screen_width - restart_width) * 0.5F;
	const float restart_y = pairs_y + sub_font_size + 30.0F;

	DrawText(restart_text.c_str(), static_cast<int>(restart_x), static_cast<int>(restart_y), restart_font_size, make_color(100, 100, 100, 255));
}
