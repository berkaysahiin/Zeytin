module;

#include "preparser.h"

export module zeytin.game.card_board_system;
import zeytin.component;

export struct CCardBoardSystem final : public Component
{
    PROPERTY(GROUP="Card Size")
    float initial_collider_width = 120.0F;

    PROPERTY(GROUP="Card Size")
    float initial_collider_height = 160.0F;

    void on_play_start() override;
    void on_play_update() override;
    void on_update() override;

    float get_reveal_timer() const { return m_reveal_timer; }
    bool is_revealed() const { return m_revealed; }

private:
    float m_reveal_timer = 0.0F;
    bool m_revealed = false;
};
