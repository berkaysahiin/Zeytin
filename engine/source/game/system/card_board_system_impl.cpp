module zeytin.game.card_board_system;

import zeytin.query;
import zeytin.game.card_layout;
import zeytin.game.card_board_config;
import zeytin.game.card_config;
import zeytin.game.transform;
import zeytin.game.aliases;
import zeytin.entity;

void CCardBoardSystem::on_play_start() {
    const auto board_opt = Query::try_find_first<GCardBoardConfig>();
    const auto card_opt = Query::try_find_first<GCardConfig>();
    if (!board_opt || !card_opt) {
        return;
    }

    clear_card_layout();
    setup_card_layout(board_opt->get(), card_opt->get(), CardAlias{});
}
