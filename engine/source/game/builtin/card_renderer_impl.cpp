module;

#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <numbers>

module zeytin.game.card_renderer;
import zeytin.raylib;
import zeytin.game.transform;
import zeytin.game.collider;
import zeytin.game.card;
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

        const float smile_start_x = center.x - (half_width * 0.15F);
        const float smile_start_y = center.y + (half_height * 0.25F);
        const float smile_end_x = center.x + (half_width * 0.15F);
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

        const float frown_start_x = center.x - (half_width * 0.2F);
        const float frown_start_y = center.y + (half_height * 0.35F);
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

    void draw_symbol_0(const Vector2 center, const float half_width, const float half_height, const Color color) {
        const float radius = half_height * 0.4F;
        const Color symbol_color = make_color(255, 100, 100, 255);
        DrawCircleV(center, radius, symbol_color);
    }

    void draw_symbol_1(const Vector2 center, const float half_width, const float half_height, const Color color) {
        const float size = half_height * 0.6F;
        const float half_size = size * 0.5F;
        const Color symbol_color = make_color(100, 200, 255, 255);
        DrawRectangleV(
            Vector2{.x=center.x - half_size, .y=center.y - half_size},
            Vector2{.x=size, .y=size},
            symbol_color
        );
    }

    void draw_symbol_2(const Vector2 center, const float half_width, const float half_height, const Color color) {
        const float radius = half_height * 0.35F;
        const Color symbol_color = make_color(100, 255, 100, 255);
        DrawCircleV(center, radius, symbol_color);
        DrawCircleV(center, radius * 0.5F, make_color(255, 255, 255, 255));
    }

    void draw_symbol_3(const Vector2 center, const float half_width, const float half_height, const Color color) {
        const float size = half_height * 0.5F;
        const Color symbol_color = make_color(255, 255, 100, 255);
        DrawRectangleV(
            Vector2{.x=center.x - size * 0.5F, .y=center.y - size * 0.3F},
            Vector2{.x=size, .y=size * 0.6F},
            symbol_color
        );
        DrawRectangleV(
            Vector2{.x=center.x - size * 0.3F, .y=center.y - size * 0.5F},
            Vector2{.x=size * 0.6F, .y=size},
            symbol_color
        );
    }

    void draw_symbol_4(const Vector2 center, const float half_width, const float half_height, const Color color) {
        const float size = half_height * 0.5F;
        const Color symbol_color = make_color(255, 150, 255, 255);
        const Vector2 points[4] = {
            Vector2{.x=center.x, .y=center.y - size},
            Vector2{.x=center.x + size, .y=center.y},
            Vector2{.x=center.x, .y=center.y + size},
            Vector2{.x=center.x - size, .y=center.y}
        };
        for (int i = 0; i < 4; ++i) {
            DrawTriangle(center, points[i], points[(i + 1) % 4], symbol_color);
        }
    }

    void draw_symbol_5(const Vector2 center, const float half_width, const float half_height, const Color color) {
        const float size = half_height * 0.45F;
        const Color symbol_color = make_color(255, 200, 100, 255);
        for (int i = 0; i < 5; ++i) {
            const float angle = -std::numbers::pi_v<float> * 0.5F + (std::numbers::pi_v<float> * 2.0F / 5.0F) * i;
            const float inner_angle = angle + std::numbers::pi_v<float> * 2.0F / 5.0F * 0.5F;
            const Vector2 outer{
                .x=center.x + size * cosf(angle),
                .y=center.y + size * sinf(angle)
            };
            const Vector2 inner{
                .x=center.x + size * 0.4F * cosf(inner_angle),
                .y=center.y + size * 0.4F * sinf(inner_angle)
            };
            const float next_angle = -std::numbers::pi_v<float> * 0.5F + (std::numbers::pi_v<float> * 2.0F / 5.0F) * ((i + 1) % 5);
            const Vector2 next_outer{
                .x=center.x + size * cosf(next_angle),
                .y=center.y + size * sinf(next_angle)
            };
            DrawTriangle(outer, inner, next_outer, symbol_color);
        }
    }

    void draw_abstract_symbol(const int symbol_id, const Vector2 center, const float half_width, const float half_height, const Color color) {
        switch (symbol_id) {
            case 0: draw_symbol_0(center, half_width, half_height, color); break;
            case 1: draw_symbol_1(center, half_width, half_height, color); break;
            case 2: draw_symbol_2(center, half_width, half_height, color); break;
            case 3: draw_symbol_3(center, half_width, half_height, color); break;
            case 4: draw_symbol_4(center, half_width, half_height, color); break;
            case 5: draw_symbol_5(center, half_width, half_height, color); break;
            default: draw_symbol_0(center, half_width, half_height, color); break;
        }
    }
}

void CCardRenderer::on_update() {
    const auto card_opt = Query::try_get<CCard>(this);
    if (!card_opt) {
        return;
    }
    const CCard& card = card_opt->get();

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

    const CardMaskStatus mask_status = static_cast<CardMaskStatus>(card.e_mask_status);
    if (mask_status == CardMaskStatus::NO_MASK) {
        draw_abstract_symbol(card.symbol_id, face_center, half_width, half_height, face_color);
    } else if (mask_status == CardMaskStatus::SMILE) {
        draw_happy_face(face_center, half_width, half_height, face_color);
    } else {
        draw_sad_face(face_center, half_width, half_height, face_color);
    }
}
