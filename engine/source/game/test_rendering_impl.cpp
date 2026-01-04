module;

#include "raylib.h"

module component.test_rendering;

void TestRendering::on_update() {
    if (!visible) {
        return;
    }
    
    Color rect_color = {
        static_cast<unsigned char>(color_r * 255),
        static_cast<unsigned char>(color_g * 255),
        static_cast<unsigned char>(color_b * 255),
        static_cast<unsigned char>(color_a * 255)
    };
    
    if (is_filled) {
        DrawRectangle(position_x, position_y, 
                     static_cast<int>(width), 
                     static_cast<int>(height), 
                     rect_color);
    } else {
        DrawRectangleLines(position_x, position_y, 
                          static_cast<int>(width), 
                          static_cast<int>(height), 
                          rect_color);
    }
    
    // Draw label if not empty
    if (!label.empty()) {
        DrawText(label.c_str(), 
                position_x + 5, 
                position_y + 5, 
                20, 
                WHITE);
    }
    
    // Draw border if not filled and thickness > 0
    if (!is_filled && border_thickness > 0) {
        DrawRectangleLinesEx(
            Rectangle{
                static_cast<float>(position_x), 
                static_cast<float>(position_y), 
                width, 
                height
            }, 
            border_thickness, 
            rect_color
        );
    }
}
