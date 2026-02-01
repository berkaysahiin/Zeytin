module;

#include "raylib.h"
#include <set>
#include <string>
#include <cmath>

module zeytin.game.matching_game;
import zeytin.query;
import zeytin.logger;
import zeytin.game.card;
import zeytin.game.card_config;
import zeytin.game.card_board_system;
import zeytin.game.transform;
import zeytin.game.collider;
import zeytin.game.card_renderer;
import zeytin.game.ui_config;
import zeytin.game.score_state;
import zeytin.game.scoring_config;
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

	void draw_corner_flames(float strength) {
		const float base_width = VIRTUAL_WIDTH > 0.0F ? VIRTUAL_WIDTH : static_cast<float>(get_screen_width());
		const float base_height = VIRTUAL_HEIGHT > 0.0F ? VIRTUAL_HEIGHT : static_cast<float>(get_screen_height());
		const float size = 120.0F + strength * 80.0F;
		const int alpha = static_cast<int>(200.0F * strength);
		const Color flame = make_color(255, 120, 40, alpha);
		const Color flame_mid = make_color(255, 170, 80, static_cast<int>(alpha * 0.7F));
		const Color flame_low = make_color(255, 220, 140, static_cast<int>(alpha * 0.45F));

		auto draw_corner = [&](float x, float y, float dx, float dy) {
			DrawTriangle(
				Vector2{.x=x, .y=y},
				Vector2{.x=x + dx * size, .y=y},
				Vector2{.x=x, .y=y + dy * size},
				flame
			);
			DrawTriangle(
				Vector2{.x=x, .y=y},
				Vector2{.x=x + dx * (size * 0.7F), .y=y},
				Vector2{.x=x, .y=y + dy * (size * 0.7F)},
				flame_mid
			);
			DrawTriangle(
				Vector2{.x=x, .y=y},
				Vector2{.x=x + dx * (size * 0.45F), .y=y},
				Vector2{.x=x, .y=y + dy * (size * 0.45F)},
				flame_low
			);
		};

		draw_corner(0.0F, 0.0F, 1.0F, 1.0F);
		draw_corner(base_width, 0.0F, -1.0F, 1.0F);
		draw_corner(0.0F, base_height, 1.0F, -1.0F);
		draw_corner(base_width, base_height, -1.0F, -1.0F);
	}

}

void GMatchingGame::on_play_start() {
	m_first_selected = 0;
	m_second_selected = 0;
	m_remaining_actions = allowed_actions;
	m_found_pairs.clear();
	m_game_over = false;
	m_matched_pairs_count = 0;
	m_game_over_win = false;
	m_game_over_timer = 0.0F;
	m_game_over_fade = 0.0F;
	m_game_over_reason = "";
	m_next_level_requested = false;
	m_current_streak = 0;
	m_current_multiplier = 1.0F;
	const auto score_opt = Query::try_find_first<GScoreState>();
	if (score_opt) {
		score_opt->get().multiplier = m_current_multiplier;
	}
	log_info("Matching game started with {} actions", m_remaining_actions);
}

void GMatchingGame::on_play_update() {
	if (m_game_over) {
		m_game_over_timer += get_frame_time();
		m_game_over_fade += get_frame_time();
		const auto score_opt = Query::try_find_first<GScoreState>();
		const std::string next_level = score_opt ? score_opt->get().next_level : "";
		if (!m_next_level_requested && m_game_over_win && !next_level.empty() && m_game_over_timer > 2.5F) {
			Zeytin::get().request_level_load(next_level);
			m_next_level_requested = true;
		}
		if (m_game_over_timer > 2.5F && (is_key_pressed(KEY_R) || is_mouse_button_pressed(0))) {
			if (m_game_over_win && !next_level.empty()) {
				Zeytin::get().request_level_load(next_level);
			} else {
				Zeytin::get().reload_scene();
			}
		}
		draw_game_over_overlay();
		return;
	}

	if (m_camera_shake_active) {
		m_camera_shake_timer += get_frame_time();
		if (m_camera_shake_duration > 0.0F && m_camera_shake_timer <= m_camera_shake_duration) {
			const float phase = m_camera_shake_timer * 60.0F;
			const float offset_x = sinf(phase * 1.7F) * m_camera_shake_amplitude;
			const float offset_y = cosf(phase * 2.1F) * m_camera_shake_amplitude;
			Camera2D& camera = Zeytin::get().get_camera();
			camera.target = Vector2{.x=m_camera_base_target.x + offset_x, .y=m_camera_base_target.y + offset_y};
		} else {
			Camera2D& camera = Zeytin::get().get_camera();
			camera.target = m_camera_base_target;
			m_camera_shake_active = false;
			m_camera_shake_timer = 0.0F;
		}
	}

	// actions are shown in score panel
	draw_score_ui();
	draw_selection_highlight();
	draw_mismatch_highlight();
	draw_move_highlight();
	draw_invalid_selection_highlight();
	if (m_multiplier_fx_active) {
		const auto ui_opt = Query::try_find_first<GGameUIConfig>();
		const float duration = ui_opt ? ui_opt->get().multiplier_fx_duration : 0.3F;
		const float decay = duration > 0.0F ? (1.0F - (m_multiplier_fx_timer / duration)) : 0.0F;
		float strength = decay;
		if (m_current_multiplier > 1.0F) {
			const float mult_strength = (m_current_multiplier - 1.0F) / 4.0F;
			strength = decay * (mult_strength > 1.0F ? 1.0F : mult_strength);
		}
		draw_corner_flames(strength);
	}
	draw_match_highlight();

	if (m_mismatch_first != 0 || m_mismatch_prev_first != 0) {
		m_mismatch_timer += get_frame_time();
		const auto ui_opt = Query::try_find_first<GGameUIConfig>();
		const float duration = ui_opt ? ui_opt->get().mismatch_duration : 0.0F;
		if (duration <= 0.0F) {
			m_mismatch_first = 0;
			m_mismatch_second = 0;
			m_mismatch_prev_first = 0;
			m_mismatch_prev_second = 0;
			m_mismatch_timer = 0.0F;
		} else if (m_mismatch_timer >= duration) {
			if (m_mismatch_prev_first == 0 && m_mismatch_first != 0) {
				m_mismatch_prev_first = m_mismatch_first;
				m_mismatch_prev_second = m_mismatch_second;
				m_mismatch_first = 0;
				m_mismatch_second = 0;
				m_mismatch_timer = 0.0F;
			} else {
				m_mismatch_prev_first = 0;
				m_mismatch_prev_second = 0;
				m_mismatch_timer = 0.0F;
			}
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

	if (m_match_to != 0) {
		m_match_timer += get_frame_time();
		const auto ui_opt = Query::try_find_first<GGameUIConfig>();
		const float duration = ui_opt ? ui_opt->get().match_trail_duration : 0.0F;
		if (duration <= 0.0F || m_match_timer >= duration) {
			m_match_from = 0;
			m_match_to = 0;
			m_match_timer = 0.0F;
		}
	}

	if (m_actions_effect_active) {
		m_actions_effect_timer += get_frame_time();
		const auto ui_opt = Query::try_find_first<GGameUIConfig>();
		const float duration = ui_opt ? ui_opt->get().actions_effect_duration : 0.25F;
		if (duration <= 0.0F || m_actions_effect_timer >= duration) {
			m_actions_effect_active = false;
			m_actions_effect_timer = 0.0F;
		}
	}

	if (m_multiplier_fx_active) {
		m_multiplier_fx_timer += get_frame_time();
		const auto ui_opt = Query::try_find_first<GGameUIConfig>();
		const float duration = ui_opt ? ui_opt->get().multiplier_fx_duration : 0.3F;
		if (duration <= 0.0F || m_multiplier_fx_timer >= duration) {
			m_multiplier_fx_active = false;
			m_multiplier_fx_timer = 0.0F;
		}
	}

	if (m_invalid_select_id != 0) {
		m_invalid_select_timer += get_frame_time();
		const auto ui_opt = Query::try_find_first<GGameUIConfig>();
		const float duration = ui_opt ? ui_opt->get().invalid_select_duration : 0.2F;
		if (duration <= 0.0F || m_invalid_select_timer >= duration) {
			m_invalid_select_id = 0;
			m_invalid_select_timer = 0.0F;
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
				m_invalid_select_id = id;
				m_invalid_select_timer = 0.0F;
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
			m_invalid_select_id = id;
			m_invalid_select_timer = 0.0F;
			return;
		}

		m_first_selected = id;
		m_first_mask_type = card.e_mask_status;
		log_info("First selected: card {}, mask type {}", id, static_cast<int>(m_first_mask_type));
	} else {
		m_second_selected = id;
		m_second_mask_type = card.e_mask_status;

		m_remaining_actions--;
		m_actions_effect_active = true;
		m_actions_effect_timer = 0.0F;

		auto& first_card = Query::get<CCard>(m_first_selected);

		if (card.e_mask_status != CardMaskStatus::NO_MASK) {
			if (first_card.symbol_id == card.symbol_id) {
				first_card.e_mask_status = CardMaskStatus::NO_MASK;
				card.e_mask_status = CardMaskStatus::NO_MASK;
				m_found_pairs.insert(first_card.symbol_id);
				m_matched_pairs_count++;
				m_current_streak++;
				const auto scoring_opt = Query::try_find_first<GScoringConfig>();
				const int base_points = scoring_opt ? scoring_opt->get().base_match_points : 10;
				const float step = scoring_opt ? scoring_opt->get().multiplier_step : 0.5F;
				const float max_mult = scoring_opt ? scoring_opt->get().max_multiplier : 5.0F;
				const float bonus = scoring_opt ? scoring_opt->get().sad_happy_bonus : 1.0F;

			m_current_multiplier += step;
			if (m_current_multiplier > max_mult) {
				m_current_multiplier = max_mult;
			}

				const bool is_sad_happy = (m_first_mask_type != m_second_mask_type) &&
					((m_first_mask_type == CardMaskStatus::SMILE && m_second_mask_type == CardMaskStatus::SAD) ||
					 (m_first_mask_type == CardMaskStatus::SAD && m_second_mask_type == CardMaskStatus::SMILE));
				const float effective_multiplier = m_current_multiplier + (is_sad_happy ? bonus : 0.0F);

				const auto score_opt = Query::try_find_first<GScoreState>();
				if (score_opt) {
					auto& score = score_opt->get();
					score.current_points += static_cast<int>(static_cast<float>(base_points) * effective_multiplier);
					score.multiplier = m_current_multiplier;
				}

				if (m_current_multiplier > m_last_multiplier) {
					m_multiplier_fx_active = true;
					m_multiplier_fx_timer = 0.0F;
					m_last_multiplier = m_current_multiplier;
					const auto ui_opt = Query::try_find_first<GGameUIConfig>();
					if (ui_opt) {
						m_camera_shake_duration = 0.08F;
						m_camera_shake_amplitude = ui_opt->get().multiplier_fx_shake_px;
						m_camera_shake_timer = 0.0F;
						m_camera_shake_active = true;
						m_camera_base_target = Zeytin::get().get_camera().target;
					}
				}

				m_match_from = m_first_selected;
				m_match_to = id;
				m_match_timer = 0.0F;
				log_info("Match found: symbol {}", first_card.symbol_id);
			} else {
				first_card.e_mask_status = m_first_mask_type;
				card.e_mask_status = m_second_mask_type;
				m_current_streak = 0;
				m_current_multiplier = 1.0F;
				m_last_multiplier = m_current_multiplier;
				const auto score_opt = Query::try_find_first<GScoreState>();
				if (score_opt) {
					score_opt->get().multiplier = m_current_multiplier;
				}
				m_mismatch_first = m_first_selected;
				m_mismatch_second = id;
				m_mismatch_timer = 0.0F;
			m_mismatch_prev_first = 0;
			m_mismatch_prev_second = 0;
			const auto ui_opt = Query::try_find_first<GGameUIConfig>();
			if (ui_opt) {
				const auto& ui = ui_opt->get();
				m_camera_shake_duration = ui.mismatch_camera_shake_duration;
				m_camera_shake_amplitude = ui.mismatch_camera_shake_px;
				m_camera_shake_timer = 0.0F;
				m_camera_shake_active = true;
				m_camera_base_target = Zeytin::get().get_camera().target;
			}
			log_info("No match: symbols {} vs {}", first_card.symbol_id, card.symbol_id);
			}
		} else {
			first_card.e_mask_status = CardMaskStatus::NO_MASK;
			card.e_mask_status = m_first_mask_type;
			m_current_streak = 0;
			m_current_multiplier = 1.0F;
			m_last_multiplier = m_current_multiplier;
			const auto score_opt = Query::try_find_first<GScoreState>();
			if (score_opt) {
				score_opt->get().multiplier = m_current_multiplier;
			}
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
	const int total_cards = static_cast<int>(Query::find_all_with<CCard>().size());
	if (total_cards % 2 != 0) {
		log_error("Odd number of cards detected: {}", total_cards);
	}
	const int total_pairs = total_cards / 2;
	const bool all_found = m_matched_pairs_count >= total_pairs && total_pairs > 0;

	if (all_found) {
		m_game_over = true;
		m_game_over_win = true;
		m_game_over_timer = 0.0F;
		m_game_over_fade = 0.0F;
		m_game_over_reason = "All pairs matched";
		log_info("YOU WIN! All pairs found in {} actions", allowed_actions - m_remaining_actions);
	} else if (m_remaining_actions <= 0) {
		m_game_over = true;
		m_game_over_win = false;
		m_game_over_timer = 0.0F;
		m_game_over_fade = 0.0F;
		m_game_over_reason = "Out of actions";
		log_info("GAME OVER. Found {}/{} pairs", m_matched_pairs_count, total_pairs);
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
	const EntityID trail_from = m_mismatch_prev_first != 0 ? m_mismatch_prev_first : m_mismatch_first;
	const EntityID trail_to = m_mismatch_prev_second != 0 ? m_mismatch_prev_second : m_mismatch_second;
	if (trail_from == 0 || trail_to == 0) {
		return;
	}

	const auto ui_opt = Query::try_find_first<GGameUIConfig>();
	if (!ui_opt) {
		return;
	}
	const auto& ui = ui_opt->get();

	const float duration = ui.mismatch_duration;
	const float alpha = duration > 0.0F ? (1.0F - (m_mismatch_timer / duration)) : 0.0F;
	const float clamped = alpha < 0.0F ? 0.0F : (alpha > 1.0F ? 1.0F : alpha);
	const int a = static_cast<int>(static_cast<float>(ui.mismatch_color_a) * clamped);
	const Color mismatch_color = make_color(ui.mismatch_color_r, ui.mismatch_color_g, ui.mismatch_color_b, a);

	const auto& from_transform = Query::get<CTransform>(trail_from);
	const auto& to_transform = Query::get<CTransform>(trail_to);
	const Vector2 from{.x=from_transform.position_x, .y=from_transform.position_y};
	const Vector2 to{.x=to_transform.position_x, .y=to_transform.position_y};

	const float base_thickness = ui.mismatch_trail_thickness;
	for (int i = 2; i >= 0; --i) {
		const float layer = static_cast<float>(i);
		const float thickness = base_thickness + layer * 3.0F;
		const float layer_alpha = clamped * (0.35F - layer * 0.1F);
		const int layer_a = static_cast<int>(static_cast<float>(ui.mismatch_color_a) * layer_alpha);
		const Color glow_color = make_color(ui.mismatch_color_r, ui.mismatch_color_g, ui.mismatch_color_b, layer_a);
		DrawLineEx(from, to, thickness, glow_color);
	}
	DrawLineEx(from, to, base_thickness, mismatch_color);
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

void GMatchingGame::draw_match_highlight() {
	if (m_match_to == 0 || m_match_from == 0) {
		return;
	}

	const auto ui_opt = Query::try_find_first<GGameUIConfig>();
	if (!ui_opt) {
		return;
	}
	const auto& ui = ui_opt->get();

	const float duration = ui.match_trail_duration;
	const float alpha = duration > 0.0F ? (1.0F - (m_match_timer / duration)) : 0.0F;
	const float clamped = alpha < 0.0F ? 0.0F : (alpha > 1.0F ? 1.0F : alpha);
	const int a = static_cast<int>(static_cast<float>(ui.match_color_a) * clamped);
	const Color match_color = make_color(ui.match_color_r, ui.match_color_g, ui.match_color_b, a);

	const auto& from_transform = Query::get<CTransform>(m_match_from);
	const auto& to_transform = Query::get<CTransform>(m_match_to);
	const Vector2 from{.x=from_transform.position_x, .y=from_transform.position_y};
	const Vector2 to{.x=to_transform.position_x, .y=to_transform.position_y};

	const float base_thickness = ui.match_trail_thickness;
	for (int i = 2; i >= 0; --i) {
		const float layer = static_cast<float>(i);
		const float thickness = base_thickness + layer * 3.0F;
		const float layer_alpha = clamped * (0.35F - layer * 0.1F);
		const int layer_a = static_cast<int>(static_cast<float>(ui.match_color_a) * layer_alpha);
		const Color glow_color = make_color(ui.match_color_r, ui.match_color_g, ui.match_color_b, layer_a);
		DrawLineEx(from, to, thickness, glow_color);
	}
	DrawLineEx(from, to, base_thickness, match_color);
}

void GMatchingGame::draw_invalid_selection_highlight() {
	if (m_invalid_select_id == 0) {
		return;
	}

	const auto ui_opt = Query::try_find_first<GGameUIConfig>();
	if (!ui_opt) {
		return;
	}
	const auto& ui = ui_opt->get();

	const float duration = ui.invalid_select_duration;
	const float alpha = duration > 0.0F ? (1.0F - (m_invalid_select_timer / duration)) : 0.0F;
	const float clamped = alpha < 0.0F ? 0.0F : (alpha > 1.0F ? 1.0F : alpha);
	const int a = static_cast<int>(static_cast<float>(ui.invalid_select_color_a) * clamped);
	const Color color = make_color(ui.invalid_select_color_r, ui.invalid_select_color_g, ui.invalid_select_color_b, a);

	const float shake = ui.invalid_select_shake_px;
	const float phase = m_invalid_select_timer * 60.0F;
	const float offset_x = sinf(phase * 1.9F) * shake;
	const float offset_y = cosf(phase * 2.2F) * shake;
	const float thickness = ui.invalid_select_outline_thickness;

	const auto& transform = Query::get<CTransform>(m_invalid_select_id);
	const auto& collider = Query::get<CCollider>(m_invalid_select_id);
	const Rectangle rect{
		.x=transform.position_x - collider.width / 2.0F - thickness + offset_x,
		.y=transform.position_y - collider.height / 2.0F - thickness + offset_y,
		.width=collider.width + thickness * 2.0F,
		.height=collider.height + thickness * 2.0F
	};
	DrawRectangleLinesEx(rect, thickness, color);
}

void GMatchingGame::draw_actions_ui() {
	const auto ui_opt = Query::try_find_first<GGameUIConfig>();
	const auto config_opt = Query::try_find_first<GCardConfig>();
	const auto board_opt = Query::try_find_first<CCardBoardSystem>();
	if (config_opt && board_opt) {
		const auto& config = config_opt->get();
		const auto& board = board_opt->get();
		const float reveal_cutoff = config.initial_reveal_duration + config.reveal_hold_after_timer;
		if (board.get_reveal_timer() < reveal_cutoff) {
			return;
		}
	}

	const int font_size = ui_opt ? ui_opt->get().actions_font_size : 24;
	const std::string text = "Actions: " + std::to_string(m_remaining_actions);
	const int text_width = MeasureText(text.c_str(), font_size);
	const float margin_x = ui_opt ? ui_opt->get().actions_x : 20.0F;
	const float margin_y = ui_opt ? ui_opt->get().actions_y : 20.0F;
	const bool anchor_right = ui_opt ? ui_opt->get().actions_anchor_right : true;
	const float effect_duration = ui_opt ? ui_opt->get().actions_effect_duration : 0.25F;
	const float shake_px = ui_opt ? ui_opt->get().actions_shake_px : 3.0F;

	const float base_width = VIRTUAL_WIDTH > 0.0F ? VIRTUAL_WIDTH : static_cast<float>(get_screen_width());
	float x = anchor_right ? (base_width - text_width - margin_x) : margin_x;
	float y = margin_y;
	if (m_actions_effect_active && effect_duration > 0.0F) {
		const float t = m_actions_effect_timer;
		const float phase = t * 60.0F;
		const float decay = 1.0F - (t / effect_duration);
		x += sinf(phase * 1.9F) * shake_px * decay;
		y += cosf(phase * 2.3F) * shake_px * decay;
	}

	Color text_color = make_color(200, 200, 200, 255);
	if (m_actions_effect_active && effect_duration > 0.0F) {
		const float t = m_actions_effect_timer;
		const float decay = 1.0F - (t / effect_duration);
		const int r = ui_opt ? ui_opt->get().actions_flash_r : 230;
		const int g = ui_opt ? ui_opt->get().actions_flash_g : 90;
		const int b = ui_opt ? ui_opt->get().actions_flash_b : 90;
		const int base_r = 200;
		const int base_g = 200;
		const int base_b = 200;
		text_color = make_color(
			static_cast<int>(base_r + (r - base_r) * decay),
			static_cast<int>(base_g + (g - base_g) * decay),
			static_cast<int>(base_b + (b - base_b) * decay),
			255
		);
	}
	DrawText(text.c_str(), static_cast<int>(x), static_cast<int>(y), font_size, text_color);
}

void GMatchingGame::draw_score_ui() {
	const auto score_opt = Query::try_find_first<GScoreState>();
	if (!score_opt) {
		return;
	}
	const auto& score = score_opt->get();

	const auto ui_opt = Query::try_find_first<GGameUIConfig>();
	const float margin_bottom = 24.0F;
	const float padding = 18.0F;
	const float gap = 16.0F;
	const int value_size = 56;
	const int meta_size = 18;
	const float min_panel_height = padding * 2.0F + static_cast<float>(meta_size + 6 + value_size + 6);
	float panel_height = ui_opt ? ui_opt->get().score_panel_height : min_panel_height;
	if (panel_height < min_panel_height) {
		panel_height = min_panel_height;
	}

	const Color panel_color = make_color(18, 18, 22, 210);
	const Color text_color = make_color(230, 230, 230, 255);
	const Color accent_color = make_color(255, 210, 120, 255);
	const Color score_color = make_color(90, 170, 255, 255);
	const Color target_color = make_color(235, 90, 90, 255);

	const float base_width = VIRTUAL_WIDTH > 0.0F ? VIRTUAL_WIDTH : static_cast<float>(get_screen_width());
	const float base_height = VIRTUAL_HEIGHT > 0.0F ? VIRTUAL_HEIGHT : static_cast<float>(get_screen_height());
	const float horizontal_inset = 120.0F;
	const float x = horizontal_inset;
	const float width = base_width - horizontal_inset * 2.0F;
	const float y = base_height - panel_height - margin_bottom;

	DrawRectangleRec(Rectangle{.x=x, .y=y, .width=width, .height=panel_height}, panel_color);
	DrawRectangleLinesEx(Rectangle{.x=x, .y=y, .width=width, .height=panel_height}, 2.0F, make_color(255, 255, 255, 30));

	const std::string level_text = "Level " + std::to_string(score.level_index);
	const int level_width = MeasureText(level_text.c_str(), meta_size);
	DrawText(level_text.c_str(), static_cast<int>(x + width - padding - level_width), static_cast<int>(y + padding), meta_size, text_color);

	const float content_y = y + padding;
	const float value_y = content_y + static_cast<float>(meta_size + 6);
	const float cell_width = (width - padding * 2.0F - gap * 4.0F) / 5.0F;

	auto draw_cell = [&](int index, const std::string& label, const std::string& value, Color value_color) {
		const float cell_x = x + padding + (cell_width + gap) * static_cast<float>(index);
		const int label_width = MeasureText(label.c_str(), meta_size);
		const int value_width = MeasureText(value.c_str(), value_size);
		const float label_x = cell_x + (cell_width - static_cast<float>(label_width)) * 0.5F;
		const float value_x = cell_x + (cell_width - static_cast<float>(value_width)) * 0.5F;
		float draw_x = value_x;
		float draw_y = value_y;
		if (index == 2 && m_multiplier_fx_active && ui_opt) {
			const float duration = ui_opt->get().multiplier_fx_duration;
			const float t = m_multiplier_fx_timer;
			const float decay = duration > 0.0F ? (1.0F - (t / duration)) : 0.0F;
			const float shake = ui_opt->get().multiplier_fx_shake_px;
			const float phase = t * 60.0F;
			draw_x += sinf(phase * 1.7F) * shake * decay;
			draw_y += cosf(phase * 2.1F) * shake * decay;
			const int a = static_cast<int>(static_cast<float>(ui_opt->get().multiplier_fx_color_a) * decay);
			const Color glow = make_color(
				ui_opt->get().multiplier_fx_color_r,
				ui_opt->get().multiplier_fx_color_g,
				ui_opt->get().multiplier_fx_color_b,
				a
			);
			const Rectangle glow_rect{
				.x=cell_x + 6.0F,
				.y=content_y - 4.0F,
				.width=cell_width - 12.0F,
				.height=panel_height - padding * 2.0F + 8.0F
			};
			DrawRectangleRec(glow_rect, glow);

			const float flame_base = content_y + static_cast<float>(meta_size + 2);
			const float flame_height = static_cast<float>(value_size) * 0.9F;
			for (int i = 0; i < 3; ++i) {
				const float offset = static_cast<float>(i) * 10.0F;
				const float sway = sinf(phase * (1.3F + i * 0.2F)) * 4.0F;
				const float height = flame_height * (0.6F + 0.2F * sinf(phase * (1.7F + i * 0.3F)));
				const float base_x = cell_x + cell_width * 0.5F + sway + offset - 10.0F;
				const float base_y = flame_base + 6.0F;
				const float top_y = base_y - height;
				const Color flame_color = make_color(255, 120 + i * 20, 40, static_cast<int>(180 * decay));
				DrawTriangle(
					Vector2{.x=base_x, .y=base_y},
					Vector2{.x=base_x + 20.0F, .y=base_y},
					Vector2{.x=base_x + 10.0F, .y=top_y},
					flame_color
				);
			}
		}
		DrawText(label.c_str(), static_cast<int>(label_x), static_cast<int>(content_y), meta_size, text_color);
		DrawText(value.c_str(), static_cast<int>(draw_x), static_cast<int>(draw_y), value_size, value_color);
	};

	draw_cell(0, "Score", std::to_string(score.current_points), score_color);
	draw_cell(1, "Target", std::to_string(score.target_points), target_color);
	draw_cell(2, "Mult", "x" + std::to_string(score.multiplier), accent_color);
	draw_cell(3, "Pairs", std::to_string(m_found_pairs.size()), text_color);
	draw_cell(4, "Actions", std::to_string(m_remaining_actions), text_color);

	const float bar_x = x + padding;
	const float bar_y = y + panel_height - padding - 10.0F;
	const float bar_w = width - padding * 2.0F;
	const float bar_h = 6.0F;
	const float progress = score.target_points > 0
		? std::min(1.0F, static_cast<float>(score.current_points) / static_cast<float>(score.target_points))
		: 0.0F;
	DrawRectangleRec(Rectangle{.x=bar_x, .y=bar_y, .width=bar_w, .height=bar_h}, make_color(60, 60, 70, 255));
	DrawRectangleRec(Rectangle{.x=bar_x, .y=bar_y, .width=bar_w * progress, .height=bar_h}, score_color);
	DrawRectangleLinesEx(Rectangle{.x=bar_x, .y=bar_y, .width=bar_w, .height=bar_h}, 1.0F, make_color(255, 255, 255, 40));
}

void GMatchingGame::draw_game_over_overlay() {
	const float time = m_game_over_fade;
	const float pulse = 0.9F + 0.1F * sinf(time * 3.0F);
	const int font_size = static_cast<int>(56.0F * pulse);
	const float screen_width = VIRTUAL_WIDTH > 0.0F ? VIRTUAL_WIDTH : static_cast<float>(get_screen_width());
	const float screen_height = VIRTUAL_HEIGHT > 0.0F ? VIRTUAL_HEIGHT : static_cast<float>(get_screen_height());

	const int total_unique_symbols = 5;
	const bool all_found = m_found_pairs.size() == total_unique_symbols;

	const std::string message = m_game_over_win ? "LEVEL COMPLETE!" : "GAME OVER";
	const Color text_color = m_game_over_win ? make_color(120, 220, 140, 255) : make_color(230, 90, 90, 255);

	const int text_width = MeasureText(message.c_str(), font_size);
	const float x = (screen_width - text_width) * 0.5F;
	const float y = screen_height * 0.35F;

	const float bg_alpha = std::min(m_game_over_fade / 0.5F, 1.0F);
	DrawRectangle(0, 0, static_cast<int>(screen_width), static_cast<int>(screen_height), make_color(10, 10, 12, static_cast<int>(200.0F * bg_alpha)));

	if (m_game_over_fade >= 0.3F) {
		const float text_alpha = std::min((m_game_over_fade - 0.3F) / 0.5F, 1.0F);
		DrawText(message.c_str(), static_cast<int>(x + 4), static_cast<int>(y + 4), font_size, make_color(0, 0, 0, static_cast<int>(180.0F * text_alpha)));
		DrawText(message.c_str(), static_cast<int>(x), static_cast<int>(y), font_size, make_color(text_color.r, text_color.g, text_color.b, static_cast<int>(255.0F * text_alpha)));

		if (!m_game_over_reason.empty() && m_game_over_fade >= 0.7F) {
			const float reason_alpha = std::min((m_game_over_fade - 0.7F) / 0.5F, 1.0F);
			const int reason_size = 24;
			const int reason_width = MeasureText(m_game_over_reason.c_str(), reason_size);
			const float reason_x = (screen_width - reason_width) * 0.5F;
			const float reason_y = screen_height * 0.5F;
			DrawText(m_game_over_reason.c_str(), static_cast<int>(reason_x), static_cast<int>(reason_y), reason_size, make_color(220, 180, 120, static_cast<int>(255.0F * reason_alpha)));
		}

		if (m_game_over_fade >= 2.0F) {
			const float prompt_alpha = std::min((m_game_over_fade - 2.0F) / 0.5F, 1.0F);
			const int prompt_size = 20;
			const char* prompt_text = "Press R or click to restart";
			const int prompt_width = MeasureText(prompt_text, prompt_size);
			const float prompt_x = (screen_width - prompt_width) * 0.5F;
			const float prompt_y = screen_height * 0.7F;
			const float blink = (sinf(m_game_over_fade * 5.0F) * 0.5F + 0.5F) * prompt_alpha;
			DrawText(prompt_text, static_cast<int>(prompt_x), static_cast<int>(prompt_y), prompt_size, make_color(230, 230, 230, static_cast<int>(255.0F * blink)));
		}
	}
}
