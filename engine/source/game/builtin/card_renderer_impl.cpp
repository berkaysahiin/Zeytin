module;

#include "raylib.h"
#include <algorithm>

module zeytin.game.card_renderer;
import zeytin.raylib;
import zeytin.game.transform;
import zeytin.game.collider;
import zeytin.query;

namespace {
    Color make_color(const int red, const int green, const int blue, const int alpha) {
        const int clamped_red = std::clamp(red, 0, 255);
        const int clamped_green = std::clamp(green, 0, 255);
        const int clamped_blue = std::clamp(blue, 0, 255);
        const int clamped_alpha = std::clamp(alpha, 0, 255);
        return Color{
            .r=static_cast<unsigned char>(clamped_red),
            .g=static_cast<unsigned char>(clamped_green),
            .b=static_cast<unsigned char>(clamped_blue),
            .a=static_cast<unsigned char>(clamped_alpha)
        };
    }

    void draw_happy_face(const Vector2 center, const float half_width, const float half_height, const Color face_color) {
        const float eye_offset_x = half_width * 0.25F;
        const float eye_offset_y = half_height * 0.15F;
        const float eye_radius = half_height * 0.1F;
        const Color eye_color{.r=0, .g=0, .b=0, .a=255};

        DrawCircleV(Vector2{.x=center.x - eye_offset_x, .y=center.y - eye_offset_y}, eye_radius, eye_color);
        DrawCircleV(Vector2{.x=center.x + eye_offset_x, .y=center.y - eye_offset_y}, eye_radius, eye_color);

        const float smile_start_x = center.x - half_width * 0.15F;
        const float smile_start_y = center.y + half_height * 0.25F;
        const float smile_end_x = center.x + half_width * 0.15F;
        const float smile_end_y = smile_start_y;
        const float smile_thickness = half_height * 0.08F;

        draw_line_ex(
            Vector2{.x=smile_start_x, .y=smile_start_y},
            Vector2{.x=smile_end_x, .y=smile_end_y},
            smile_thickness,
            eye_color
        );
    }

    void draw_sad_face(const Vector2 center, const float half_width, const float half_height, const Color face_color) {
        const float eye_offset_x = half_width * 0.25F;
        const float eye_offset_y = half_height * 0.15F;
        const float eye_radius = half_height * 0.1F;
        const Color eye_color{.r=0, .g=0, .b=0, .a=255};

        DrawCircleV(Vector2{.x=center.x - eye_offset_x, .y=center.y - eye_offset_y}, eye_radius, eye_color);
        DrawCircleV(Vector2{.x=center.x + eye_offset_x, .y=center.y - eye_offset_y}, eye_radius, eye_color);

        const float frown_start_x = center.x - half_width * 0.2F;
        const float frown_start_y = center.y + half_height * 0.35F;
        const float frown_end_x = center.x + half_width * 0.2F;
        const float frown_end_y = frown_start_y - half_height * 0.15F;
        const float frown_control_x = center.x;
        const float frown_control_y = center.y + half_height * 0.1F;
        const float frown_thickness = half_height * 0.08F;

	DrawSplineSegmentBezierQuadratic(
            Vector2{.x=frown_start_x, .y=frown_start_y},
            Vector2{.x=frown_control_x, .y=frown_control_y},
            Vector2{.x=frown_end_x, .y=frown_end_y},
            frown_thickness,
            eye_color
        );
    }
}

void CCardRenderer::on_update() {
    const auto transform_opt = Query::try_get<CTransform>(this);
    if (!transform_opt) {
        return;
    }
    const CTransform& transform = transform_opt->get();

    const auto collider_opt = Query::try_get<CCollider>(this);
    if (!collider_opt) {
        return;
    }
    const CCollider& collider = collider_opt->get();

    const Color card_color = make_color(card_red, card_green, card_blue, card_alpha);
    const Color face_color = make_color(face_red, face_green, face_blue, face_alpha);

    const float width = collider.width;
    const float height = collider.height;
    const float half_width = width * 0.5F;
    const float half_height = height * 0.5F;

    const Rectangle card_rect{
        .x=transform.position_x - half_width,
        .y=transform.position_y - half_height,
        .width=width,
        .height=height
    };

    DrawRectangleRec(card_rect, card_color);
    DrawRectangleLinesEx(card_rect, 2.0F, Color{.r=0, .g=0, .b=0, .a=255});

    const Vector2 face_center{.x=transform.position_x, .y=transform.position_y};

    if (face_type == 0) {
        draw_happy_face(face_center, half_width, half_height, face_color);
    } else {
        draw_sad_face(face_center, half_width, half_height, face_color);
    }
}
