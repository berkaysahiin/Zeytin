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
        const float mask_scale = 0.75F;
        const float mask_half_width = half_width * mask_scale * 0.7F;
        const float mask_half_height = half_height * mask_scale;

        const Color mask_bg_color = make_color(230, 230, 230, 255);
        const Color mask_outline_color = make_color(40, 40, 40, 255);
        const Color feature_color = make_color(30, 30, 30, 255);
        const Color decor_color = make_color(180, 180, 180, 255);

        const float top_y = center.y - mask_half_height * 0.9F;
        const float bottom_y = center.y + mask_half_height * 0.95F;
        const float left_x = center.x - mask_half_width;
        const float right_x = center.x + mask_half_width;

        const float shoulder_width = mask_half_width * 1.2F;

        Vector2 points[12] = {
            Vector2{.x=center.x - shoulder_width, .y=bottom_y * 0.5F + center.y * 0.5F},
            Vector2{.x=center.x - mask_half_width * 0.8F, .y=bottom_y * 0.7F + center.y * 0.3F},
            Vector2{.x=center.x - mask_half_width * 0.5F, .y=bottom_y * 0.85F + center.y * 0.15F},
            Vector2{.x=center.x, .y=bottom_y},
            Vector2{.x=center.x + mask_half_width * 0.5F, .y=bottom_y * 0.85F + center.y * 0.15F},
            Vector2{.x=center.x + mask_half_width * 0.8F, .y=bottom_y * 0.7F + center.y * 0.3F},
            Vector2{.x=center.x + shoulder_width, .y=bottom_y * 0.5F + center.y * 0.5F},
            Vector2{.x=center.x + mask_half_width, .y=center.y - mask_half_height * 0.5F},
            Vector2{.x=center.x + mask_half_width * 0.9F, .y=top_y + mask_half_height * 0.2F},
            Vector2{.x=center.x, .y=top_y},
            Vector2{.x=center.x - mask_half_width * 0.9F, .y=top_y + mask_half_height * 0.2F},
            Vector2{.x=center.x - mask_half_width, .y=center.y - mask_half_height * 0.5F}
        };

        DrawTriangleFan(points, 12, mask_bg_color);

        for (int i = 0; i < 12; ++i) {
            const Vector2 p1 = points[i];
            const Vector2 p2 = points[(i + 1) % 12];
            DrawLineEx(p1, p2, 3.0F, mask_outline_color);
        }

        const float decor_x_left = center.x - mask_half_width * 1.3F;
        const float decor_x_right = center.x + mask_half_width * 1.3F;

        DrawSplineSegmentBezierQuadratic(
            Vector2{.x=decor_x_left, .y=center.y - mask_half_height * 0.6F},
            Vector2{.x=decor_x_left - 10.0F, .y=center.y},
            Vector2{.x=decor_x_left, .y=center.y + mask_half_height * 0.4F},
            2.0F,
            decor_color
        );

        DrawSplineSegmentBezierQuadratic(
            Vector2{.x=decor_x_right, .y=center.y - mask_half_height * 0.6F},
            Vector2{.x=decor_x_right + 10.0F, .y=center.y},
            Vector2{.x=decor_x_right, .y=center.y + mask_half_height * 0.4F},
            2.0F,
            decor_color
        );

        const float eye_offset_x = mask_half_width * 0.35F;
        const float eye_offset_y = mask_half_height * 0.15F;
        const float eye_radius = mask_half_height * 0.1F;

        DrawCircleV(Vector2{.x=center.x - eye_offset_x, .y=center.y - eye_offset_y}, eye_radius, feature_color);
        DrawCircleV(Vector2{.x=center.x + eye_offset_x, .y=center.y - eye_offset_y}, eye_radius, feature_color);

        const float smile_width = mask_half_width * 0.35F;
        const float smile_y = center.y + mask_half_height * 0.15F;
        const float smile_thickness = mask_half_height * 0.05F;

        DrawSplineSegmentBezierQuadratic(
            Vector2{.x=center.x - smile_width, .y=smile_y + 8.0F},
            Vector2{.x=center.x, .y=smile_y - 12.0F},
            Vector2{.x=center.x + smile_width, .y=smile_y + 8.0F},
            smile_thickness,
            feature_color
        );
    }

    void draw_sad_face(const Vector2 center, const float half_width, const float half_height, const Color face_color) {
        const float mask_scale = 0.75F;
        const float mask_half_width = half_width * mask_scale * 0.7F;
        const float mask_half_height = half_height * mask_scale;

        const Color mask_bg_color = make_color(225, 225, 225, 255);
        const Color mask_outline_color = make_color(40, 40, 40, 255);
        const Color feature_color = make_color(30, 30, 30, 255);
        const Color decor_color = make_color(170, 170, 170, 255);

        const float top_y = center.y - mask_half_height * 0.9F;
        const float bottom_y = center.y + mask_half_height * 0.95F;
        const float left_x = center.x - mask_half_width;
        const float right_x = center.x + mask_half_width;

        const float shoulder_width = mask_half_width * 1.2F;

        Vector2 points[12] = {
            Vector2{.x=center.x - shoulder_width, .y=bottom_y * 0.5F + center.y * 0.5F},
            Vector2{.x=center.x - mask_half_width * 0.8F, .y=bottom_y * 0.7F + center.y * 0.3F},
            Vector2{.x=center.x - mask_half_width * 0.5F, .y=bottom_y * 0.85F + center.y * 0.15F},
            Vector2{.x=center.x, .y=bottom_y},
            Vector2{.x=center.x + mask_half_width * 0.5F, .y=bottom_y * 0.85F + center.y * 0.15F},
            Vector2{.x=center.x + mask_half_width * 0.8F, .y=bottom_y * 0.7F + center.y * 0.3F},
            Vector2{.x=center.x + shoulder_width, .y=bottom_y * 0.5F + center.y * 0.5F},
            Vector2{.x=center.x + mask_half_width, .y=center.y - mask_half_height * 0.5F},
            Vector2{.x=center.x + mask_half_width * 0.9F, .y=top_y + mask_half_height * 0.2F},
            Vector2{.x=center.x, .y=top_y},
            Vector2{.x=center.x - mask_half_width * 0.9F, .y=top_y + mask_half_height * 0.2F},
            Vector2{.x=center.x - mask_half_width, .y=center.y - mask_half_height * 0.5F}
        };

        DrawTriangleFan(points, 12, mask_bg_color);

        for (int i = 0; i < 12; ++i) {
            const Vector2 p1 = points[i];
            const Vector2 p2 = points[(i + 1) % 12];
            DrawLineEx(p1, p2, 3.0F, mask_outline_color);
        }

        const float decor_x_left = center.x - mask_half_width * 1.3F;
        const float decor_x_right = center.x + mask_half_width * 1.3F;

        DrawSplineSegmentBezierQuadratic(
            Vector2{.x=decor_x_left, .y=center.y - mask_half_height * 0.6F},
            Vector2{.x=decor_x_left - 8.0F, .y=center.y},
            Vector2{.x=decor_x_left, .y=center.y + mask_half_height * 0.4F},
            2.0F,
            decor_color
        );

        DrawSplineSegmentBezierQuadratic(
            Vector2{.x=decor_x_right, .y=center.y - mask_half_height * 0.6F},
            Vector2{.x=decor_x_right + 8.0F, .y=center.y},
            Vector2{.x=decor_x_right, .y=center.y + mask_half_height * 0.4F},
            2.0F,
            decor_color
        );

        const float eye_offset_x = mask_half_width * 0.35F;
        const float eye_offset_y = mask_half_height * 0.15F;
        const float eye_radius = mask_half_height * 0.1F;

        DrawCircleV(Vector2{.x=center.x - eye_offset_x, .y=center.y - eye_offset_y}, eye_radius, feature_color);
        DrawCircleV(Vector2{.x=center.x + eye_offset_x, .y=center.y - eye_offset_y}, eye_radius, feature_color);

        const float frown_width = mask_half_width * 0.3F;
        const float frown_y = center.y + mask_half_height * 0.15F;
        const float frown_thickness = mask_half_height * 0.05F;

        DrawSplineSegmentBezierQuadratic(
            Vector2{.x=center.x - frown_width, .y=frown_y - 6.0F},
            Vector2{.x=center.x, .y=frown_y + 10.0F},
            Vector2{.x=center.x + frown_width, .y=frown_y - 6.0F},
            frown_thickness,
            feature_color
        );

        const float tear_x = center.x + mask_half_width * 0.35F;
        const float tear_y = center.y + mask_half_height * 0.05F;
        const float tear_radius = mask_half_height * 0.04F;
        const Color tear_color = make_color(150, 170, 200, 255);

        DrawCircleV(Vector2{.x=tear_x, .y=tear_y}, tear_radius, tear_color);
    }

    void draw_symbol_0(const Vector2 center, const float half_width, const float half_height, const Color color) {
        const float radius = half_height * 0.4F;
        const Color symbol_color = make_color(180, 180, 180, 255);
        DrawCircleV(center, radius, symbol_color);
    }

    void draw_symbol_1(const Vector2 center, const float half_width, const float half_height, const Color color) {
        const float size = half_height * 0.6F;
        const float half_size = size * 0.5F;
        const Color symbol_color = make_color(200, 200, 200, 255);
        DrawRectangleV(
            Vector2{.x=center.x - half_size, .y=center.y - half_size},
            Vector2{.x=size, .y=size},
            symbol_color
        );
    }

    void draw_symbol_2(const Vector2 center, const float half_width, const float half_height, const Color color) {
        const float radius = half_height * 0.35F;
        const Color symbol_color = make_color(220, 220, 220, 255);
        DrawCircleV(center, radius, symbol_color);
        DrawCircleV(center, radius * 0.5F, make_color(255, 255, 255, 255));
    }

    void draw_symbol_3(const Vector2 center, const float half_width, const float half_height, const Color color) {
        const float size = half_height * 0.5F;
        const Color symbol_color = make_color(240, 240, 240, 255);
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
        const Color symbol_color = make_color(255, 255, 255, 255);
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
        const Color symbol_color = make_color(255, 255, 255, 255);
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
