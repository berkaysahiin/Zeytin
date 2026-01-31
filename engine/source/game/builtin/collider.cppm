module;

#include "preparser.h"

export module zeytin.game.collider;
import zeytin.component;

export struct CCollider : public Component 
{
    PROPERTY(GROUP="Size")   
	float width = 100.0F;

    PROPERTY(GROUP="Size")   
	float height = 100.0F;

    PROPERTY(GROUP="Offset") 
	float offset_x = 0.0F;

    PROPERTY(GROUP="Offset") 
	float offset_y = 0.0F;

    PROPERTY(GROUP="Debug")  
	bool enable_debug = true;

    PROPERTY(GROUP="Debug", ENABLE_IF=is_debug())  
	bool show_bounds = true;

    [[nodiscard]] 
	bool is_point_inside(float in_px, float in_py) const;

	[[nodiscard]] 
	bool is_debug() const { return enable_debug; }

    void on_update() override;
    void draw_bounds() const;
};
