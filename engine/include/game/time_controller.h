#pragma once

#include "game/gamesnophot.h"
#include "variant/variant_base.h"

class TimeController : public VariantBase {
    VARIANT(TimeController);
    
public:
    int max_history_frames = 600; PROPERTY(); // 10 seconds at 60fps
    
    virtual void on_init() override;
    virtual void on_play_update() override;
    
    bool is_rewinding() const { return m_is_rewinding; }
    
private:
    std::deque<GameSnapshot> m_history;
    bool m_is_rewinding = false;
    
    void record_snapshot();
    void apply_rewind();
    bool should_rewind();
};
