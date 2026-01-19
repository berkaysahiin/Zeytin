module;

#include "preparser.h"

export module zeytin.game.collider;
import zeytin.component;

export struct CCollider : public Component 
{
    PROPERTY(GROUP="Size")   
	float width = 100.0f;

    PROPERTY(GROUP="Size")   
	float height = 100.0f;

    PROPERTY(GROUP="Offset") 
	float offset_x = 0.0f;

    PROPERTY(GROUP="Offset") 
	float offset_y = 0.0f;

    PROPERTY(GROUP="Debug")  
	bool enable_debug = true;

    PROPERTY(GROUP="Debug", ENABLE_IF=is_debug())  
	bool show_bounds = true;

    PROPERTY(GROUP="Debug", ENABLE_IF=is_debug())  
	float random_float_1 = 0;

    PROPERTY(DEPRECATED="Use random_float_1 instead", GROUP="Debug", ENABLE_IF=is_debug())  
	float random_float_2 = 0;

    void on_update() override;
    void draw_bounds() const;
    bool is_point_inside(float px, float py) const;

	bool is_debug() const { return enable_debug; }
};
