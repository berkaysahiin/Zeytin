module;

#include "preparser.h"
#include "raylib.h"
#include <set>

export module zeytin.game.matching_game;
import zeytin.component;
import zeytin.game.card;
import zeytin.entity;

export struct GMatchingGame final : public Component
{
	PROPERTY(GROUP="Gameplay")
	int allowed_actions = 20;

	void on_play_start() override;
	void on_play_update() override;

private:
	EntityID m_first_selected = 0;
	EntityID m_second_selected = 0;
	CardMaskStatus m_first_mask_type = CardMaskStatus::SMILE;
	CardMaskStatus m_second_mask_type = CardMaskStatus::SMILE;
	int m_remaining_actions = 20;
	std::set<int> m_found_pairs;
	bool m_game_over = false;
	int m_matched_pairs_count = 0;

	bool handle_mouse_click();
	void process_card_selection(EntityID id, CCard& card);
	void check_game_state();
	void draw_selection_highlight();
	void draw_mismatch_highlight();
	void draw_move_highlight();
	void draw_invalid_selection_highlight();
	void draw_match_highlight();
	void draw_score_ui();
	void draw_actions_ui();
	void draw_game_over_overlay();

	float m_mismatch_timer = 0.0F;
	EntityID m_mismatch_first = 0;
	EntityID m_mismatch_second = 0;
	EntityID m_mismatch_prev_first = 0;
	EntityID m_mismatch_prev_second = 0;

	float m_camera_shake_timer = 0.0F;
	float m_camera_shake_duration = 0.0F;
	float m_camera_shake_amplitude = 0.0F;
	Vector2 m_camera_base_target{0.0F, 0.0F};
	bool m_camera_shake_active = false;

	float m_actions_effect_timer = 0.0F;
	bool m_actions_effect_active = false;

	float m_invalid_select_timer = 0.0F;
	EntityID m_invalid_select_id = 0;

	int m_current_streak = 0;
	float m_current_multiplier = 1.0F;
	float m_last_multiplier = 1.0F;
	float m_multiplier_fx_timer = 0.0F;
	bool m_multiplier_fx_active = false;

	float m_move_timer = 0.0F;
	EntityID m_move_from = 0;
	EntityID m_move_to = 0;

	float m_match_timer = 0.0F;
	EntityID m_match_from = 0;
	EntityID m_match_to = 0;
};
