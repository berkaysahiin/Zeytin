module;
#include <optional>
#include <functional>

module zeytin.game.card_board_system;
import zeytin.query;
import zeytin.logger;
import zeytin.game.card_layout;
import zeytin.game.card_board_config;
import zeytin.game.card_config;
import zeytin.game.transform;
import zeytin.game.aliases;
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
}
