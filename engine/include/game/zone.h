#pragma once

#include "variant/variant_base.h"

class Zone : public VariantBase {
    VARIANT(Zone);
    REQUIRES(Collider);


public:
    virtual void on_play_start() override;
    virtual void on_play_update() override;
    virtual void on_play_late_update() override;

    inline std::vector<int>& get_player() { return m_in_players; }

private:
    std::vector<int> m_in_players;
};
