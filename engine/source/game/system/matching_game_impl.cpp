module;

#include "raylib.h"
#include <set>
#include <string>
#include <cmath>

module zeytin.game.matching_game;
import zeytin.query;
import zeytin.logger;
import zeytin.game.card;
import zeytin.game.transform;
import zeytin.game.collider;
import zeytin.game.card_renderer;
import zeytin.game.ui_config;
import zeytin.raylib;
import zeytin.zeytin;
import zeytin.entity;
import zeytin.resource;

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
	draw_mismatch_highlight();
	draw_move_highlight();

	if (m_mismatch_first != 0) {
		m_mismatch_timer += get_frame_time();
		const auto ui_opt = Query::try_find_first<GGameUIConfig>();
		const float duration = ui_opt ? ui_opt->get().mismatch_duration : 0.0F;
		if (duration <= 0.0F || m_mismatch_timer >= duration) {
			m_mismatch_first = 0;
			m_mismatch_second = 0;
			m_mismatch_timer = 0.0F;
		}
	}

	if (m_move_to != 0) {
		m_move_timer += get_frame_time();
		const auto ui_opt = Query::try_find_first<GGameUIConfig>();
		const float duration = ui_opt ? ui_opt->get().move_duration : 0.0F;
		if (duration <= 0.0F || m_move_timer >= duration) {
			m_move_from = 0;
			m_move_to = 0;
			m_move_timer = 0.0F;
		}
	}

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
		log_info("First selected: card {}, mask type {}", id, static_cast<int>(m_first_mask_type));
	} else {
		m_second_selected = id;
		m_second_mask_type = card.e_mask_status;

		m_remaining_actions--;

		auto& first_card = Query::get<CCard>(m_first_selected);

		if (card.e_mask_status != CardMaskStatus::NO_MASK) {
			if (first_card.symbol_id == card.symbol_id) {
				first_card.e_mask_status = CardMaskStatus::NO_MASK;
				card.e_mask_status = CardMaskStatus::NO_MASK;
				m_found_pairs.insert(first_card.symbol_id);
				log_info("Match found: symbol {}", first_card.symbol_id);
			} else {
				first_card.e_mask_status = m_first_mask_type;
				card.e_mask_status = m_second_mask_type;
				m_mismatch_first = m_first_selected;
				m_mismatch_second = id;
				m_mismatch_timer = 0.0F;
				log_info("No match: symbols {} vs {}", first_card.symbol_id, card.symbol_id);
			}
		} else {
			first_card.e_mask_status = CardMaskStatus::NO_MASK;
			card.e_mask_status = m_first_mask_type;
			m_move_from = m_first_selected;
			m_move_to = id;
			m_move_timer = 0.0F;
			log_info("Move: mask from card {} to {}", m_first_selected, id);
		}

		m_first_selected = 0;
		m_second_selected = 0;
	}
}

void GMatchingGame::check_game_state() {
	const int total_unique_symbols = 5;
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
	const auto& card = Query::get<CCard>(m_first_selected);
	const auto& renderer = Query::get<CCardRenderer>(m_first_selected);

	const float lift = 8.0F;
	const float scale = 1.08F;
	const float scaled_width = collider.width * scale;
	const float scaled_height = collider.height * scale;
	const Rectangle dest{
		.x=transform.position_x - scaled_width * 0.5F,
		.y=transform.position_y - scaled_height * 0.5F - lift,
		.width=scaled_width,
		.height=scaled_height
	};

	const Rectangle source = {
		.x=0.0F,
		.y=0.0F,
		.width=0.0F,
		.height=0.0F
	};

	const CardMaskStatus mask_status = static_cast<CardMaskStatus>(card.e_mask_status);
	const std::string* texture_path = nullptr;
	if (mask_status == CardMaskStatus::SMILE) {
		texture_path = &renderer.mask_happy_path;
	} else if (mask_status == CardMaskStatus::SAD) {
		texture_path = &renderer.mask_sad_path;
	} else {
		texture_path = (card.symbol_id == 1) ? &renderer.symbol_1_path : &renderer.symbol_0_path;
	}

	if (texture_path != nullptr) {
		ResourcePtr<Texture2D> texture(*texture_path);
		if (texture.is_valid()) {
			Texture2D* tex = texture.get_ptr();
			const Rectangle tex_source{.x=0, .y=0, .width=static_cast<float>(tex->width), .height=static_cast<float>(tex->height)};
			DrawTexturePro(*tex, tex_source, dest, Vector2{.x=0, .y=0}, 0.0F, WHITE);
		}
	}

	const float outline_thickness = 6.0F;
	const Rectangle outline{
		.x=transform.position_x - collider.width / 2.0F - outline_thickness,
		.y=transform.position_y - collider.height / 2.0F - outline_thickness,
		.width=collider.width + outline_thickness * 2.0F,
		.height=collider.height + outline_thickness * 2.0F
	};
	const Color highlight_color = make_color(255, 215, 0, 255);
	DrawRectangleLinesEx(outline, outline_thickness, highlight_color);
}

void GMatchingGame::draw_mismatch_highlight() {
	if (m_mismatch_first == 0) {
		return;
	}

	const auto ui_opt = Query::try_find_first<GGameUIConfig>();
	if (!ui_opt) {
		return;
	}
	const auto& ui = ui_opt->get();

	const float thickness = ui.mismatch_outline_thickness;
	const float shake = ui.mismatch_shake_px;
	const Color mismatch_color = make_color(ui.mismatch_color_r, ui.mismatch_color_g, ui.mismatch_color_b, ui.mismatch_color_a);
	const float t = m_mismatch_timer * 60.0F;
	const float offset_x = sinf(t * 1.7F) * shake;
	const float offset_y = cosf(t * 2.1F) * shake;

	auto draw_outline_for = [&](EntityID id) {
		const auto& transform = Query::get<CTransform>(id);
		const auto& collider = Query::get<CCollider>(id);
		const Rectangle rect{
			.x=transform.position_x - collider.width / 2.0F - thickness + offset_x,
			.y=transform.position_y - collider.height / 2.0F - thickness + offset_y,
			.width=collider.width + thickness * 2.0F,
			.height=collider.height + thickness * 2.0F
		};
		DrawRectangleLinesEx(rect, thickness, mismatch_color);
	};

	draw_outline_for(m_mismatch_first);
	if (m_mismatch_second != 0) {
		draw_outline_for(m_mismatch_second);
	}
}

void GMatchingGame::draw_move_highlight() {
	if (m_move_to == 0) {
		return;
	}

	const auto ui_opt = Query::try_find_first<GGameUIConfig>();
	if (!ui_opt) {
		return;
	}
	const auto& ui = ui_opt->get();
	if (m_move_from == 0 || ui.move_duration <= 0.0F) {
		return;
	}

	const float remaining = 1.0F - (m_move_timer / ui.move_duration);
	const float alpha = remaining < 0.0F ? 0.0F : (remaining > 1.0F ? 1.0F : remaining);
	const Color move_color = make_color(
		ui.move_color_r,
		ui.move_color_g,
		ui.move_color_b,
		static_cast<int>(static_cast<float>(ui.move_color_a) * alpha)
	);

	const auto& from_transform = Query::get<CTransform>(m_move_from);
	const auto& to_transform = Query::get<CTransform>(m_move_to);
	const Vector2 from{.x=from_transform.position_x, .y=from_transform.position_y};
	const Vector2 to{.x=to_transform.position_x, .y=to_transform.position_y};

	const float base_thickness = ui.move_outline_thickness;
	for (int i = 2; i >= 0; --i) {
		const float layer = static_cast<float>(i);
		const float thickness = base_thickness + layer * 3.0F;
		const float layer_alpha = alpha * (0.35F - layer * 0.1F);
		const int a = static_cast<int>(static_cast<float>(ui.move_color_a) * layer_alpha);
		const Color glow_color = make_color(ui.move_color_r, ui.move_color_g, ui.move_color_b, a);
		DrawLineEx(from, to, thickness, glow_color);
	}
	DrawLineEx(from, to, base_thickness, move_color);
}

void GMatchingGame::draw_actions_ui() {
	const auto ui_opt = Query::try_find_first<GGameUIConfig>();
	if (!ui_opt) {
		return;
	}
	const auto& ui = ui_opt->get();
	const int font_size = ui.actions_font_size;
	const std::string text = "Actions: " + std::to_string(m_remaining_actions);
	const int text_width = MeasureText(text.c_str(), font_size);
	const float margin_x = ui.actions_x;
	const float margin_y = ui.actions_y;
	const bool anchor_right = ui.actions_anchor_right;

	const float screen_width = static_cast<float>(get_screen_width());
	const float x = anchor_right ? (screen_width - text_width - margin_x) : margin_x;
	const float y = margin_y;

	const Color text_color = make_color(200, 200, 200, 255);
	DrawText(text.c_str(), static_cast<int>(x), static_cast<int>(y), font_size, text_color);
}

void GMatchingGame::draw_game_over_overlay() {
	const int font_size = 48;
	const float screen_width = static_cast<float>(get_screen_width());
	const float screen_height = static_cast<float>(get_screen_height());

	const int total_unique_symbols = 5;
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
