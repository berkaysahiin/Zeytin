#pragma once

#include "variant/variant_base.h"

class Zone : public VariantBase {
    VARIANT(Zone);
    REQUIRES(Collider);

public:
    virtual void on_play_start();
    inline std::vector<int>& get_player() { return m_in_players; }

private:
    std::vector<int> m_in_players;
};
