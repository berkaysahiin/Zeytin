module;

#include "preparser.h"
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

	bool handle_mouse_click();
	void process_card_selection(EntityID id, CCard& card);
	void check_game_state();
	void draw_selection_highlight();
	void draw_actions_ui();
	void draw_game_over_overlay();
};
