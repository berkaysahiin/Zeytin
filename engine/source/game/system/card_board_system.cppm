export module zeytin.game.card_board_system;
import zeytin.component;

export struct CCardBoardSystem final : public Component
{
    void on_play_start() override;
};
