module;

#include "raylib.h"
#include <cmath>
#include <numbers>
#include <string>

module zeytin.game.card_renderer;
import zeytin.raylib;
import zeytin.game.transform;
import zeytin.game.collider;
import zeytin.game.card;
import zeytin.query;
import zeytin.resource;
import zeytin.logger;

namespace {
    void draw_symbol_0(const Vector2 center, const float half_height) {
        const float radius = half_height * 0.4F;
        const Color symbol_color = Color{.r=180, .g=180, .b=180, .a=255};
        DrawCircleV(center, radius, symbol_color);
    }

    void draw_symbol_1(const Vector2 center, const float half_height) {
        const float size = half_height * 0.6F;
        const float half_size = size * 0.5F;
        const Color symbol_color = Color{.r=200, .g=200, .b=200, .a=255};
        DrawRectangleV(
            Vector2{.x=center.x - half_size, .y=center.y - half_size},
            Vector2{.x=size, .y=size},
            symbol_color
        );
    }

    void draw_symbol_2(const Vector2 center, const float half_height) {
        const float radius = half_height * 0.35F;
        const Color symbol_color = Color{.r=220, .g=220, .b=220, .a=255};
        DrawCircleV(center, radius, symbol_color);
        DrawCircleV(center, radius * 0.5F, Color{.r=255, .g=255, .b=255, .a=255});
    }

    void draw_symbol_3(const Vector2 center, const float half_height) {
        const float size = half_height * 0.5F;
        const Color symbol_color = Color{.r=240, .g=240, .b=240, .a=255};
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

    void draw_symbol_4(const Vector2 center, const float half_height) {
        const float size = half_height * 0.5F;
        const Color symbol_color = Color{.r=255, .g=255, .b=255, .a=255};
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

    void draw_symbol_5(const Vector2 center, const float half_height) {
        const float size = half_height * 0.45F;
        const Color symbol_color = Color{.r=255, .g=255, .b=255, .a=255};
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

    void draw_abstract_symbol(const int symbol_id, const Vector2 center, const float half_height) {
        switch (symbol_id) {
            case 0: draw_symbol_0(center, half_height); break;
            case 1: draw_symbol_1(center, half_height); break;
            case 2: draw_symbol_2(center, half_height); break;
            case 3: draw_symbol_3(center, half_height); break;
            case 4: draw_symbol_4(center, half_height); break;
            case 5: draw_symbol_5(center, half_height); break;
            default: draw_symbol_0(center, half_height); break;
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

    const float width = collider.width;
    const float height = collider.height;
    const float half_width = width * 0.5F;
    const float half_height = height * 0.5F;

    const Vector2 face_center{.x=transform.position_x, .y=transform.position_y};

    const auto mask_status = static_cast<CardMaskStatus>(card.e_mask_status);
    if (mask_status == CardMaskStatus::NO_MASK) {
        draw_abstract_symbol(card.symbol_id, face_center, half_height);
    } else if (mask_status == CardMaskStatus::SMILE) {
        ResourcePtr<Texture2D> happy_texture(mask_happy_path);
        if (happy_texture.is_valid()) {
            Texture2D* tex = happy_texture.get_ptr();
            const Rectangle source{.x=0, .y=0, .width=static_cast<float>(tex->width), .height=static_cast<float>(tex->height)};
            const Rectangle dest{.x=face_center.x - half_width, .y=face_center.y - (half_height / 1.2F), .width=width, .height=height};
            DrawTexturePro(*tex, source, dest, Vector2{.x=0, .y=0}, 0.0F, WHITE);
        } else {
            log_error("Couldn't find valid resource: {}", mask_happy_path);
        }
    } else {
        ResourcePtr<Texture2D> sad_texture(mask_sad_path);
        if (sad_texture.is_valid()) {
            Texture2D* tex = sad_texture.get_ptr();
            const Rectangle source{.x=0, .y=0, .width=static_cast<float>(tex->width), .height=static_cast<float>(tex->height)};
            const Rectangle dest{.x=face_center.x - half_width, .y=face_center.y - (half_height / 1.2F), .width=width, .height=height};
            DrawTexturePro(*tex, source, dest, Vector2{.x=0, .y=0}, 0.0F, WHITE);
        } else {
            log_error("Couldn't find valid resource: {}", mask_sad_path);
        }
    }
}
