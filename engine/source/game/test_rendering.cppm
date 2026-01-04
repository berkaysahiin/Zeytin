module;

#include <string>
#include "preparser.h"

export module component.test_rendering;
import zeytin.component;

export class TestRendering : public Component {
public:
    PROPERTY()
    int position_x = 100;
    
    PROPERTY()
    int position_y = 100;
    
    PROPERTY()
    float width = 200.0f;
    
    PROPERTY()
    float height = 150.0f;
    
    PROPERTY()
    float color_r = 1.0f;
    
    PROPERTY()
    float color_g = 0.0f;
    
    PROPERTY()
    float color_b = 0.0f;
    
    PROPERTY()
    float color_a = 1.0f;
    
    PROPERTY()
    bool is_filled = true;
    
    PROPERTY()
    float border_thickness = 2.0f;
    
    PROPERTY()
    std::string label = "Test Rectangle";
    
    PROPERTY()
    bool visible = true;
    
    void on_update() override;
};
