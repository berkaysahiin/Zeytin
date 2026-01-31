module;
#include "raylib.h"
#include <algorithm>

module zeytin.game.card_renderer;
import zeytin.query;
import zeytin.game.card;
import zeytin.game.card_config;
import zeytin.game.transform;
import zeytin.game.aliases;
import zeytin.entity;

namespace {
    template<typename... Ts>
    auto find_cards(const Alias<Ts...>&) {
        static_assert(alias_contains_v<CCard, Ts...>, "Alias must include CCard");
        static_assert(alias_contains_v<CTransform, Ts...>, "Alias must include CTransform");
        return Query::find_all_with<Ts...>();
    }

    Color with_alpha(const Color base, const unsigned char alpha) {
        return Color{.r=base.r, .g=base.g, .b=base.b, .a=alpha};
    }

    void draw_stripes(const Rectangle rect, const float spacing, const Color color) {
        const float start_x = rect.x - rect.height;
        const float end_x = rect.x + rect.width + rect.height;

        for (float x = start_x; x < end_x; x += spacing) {
            const Vector2 start{ .x=x, .y=rect.y };
            const Vector2 end{ .x=x + rect.height, .y=rect.y + rect.height };
            DrawLineV(start, end, color);
        }
    }

    void draw_mask_badge(const Rectangle card_rect, const float scale) {
        const float badge_size = std::min(card_rect.width, card_rect.height) * 0.22F;
        if (badge_size < 6.0F) {
            return;
        }

        const float padding = std::max(4.0F, 6.0F * scale);
        const float badge_x = card_rect.x + card_rect.width - badge_size - padding;
        const float badge_y = card_rect.y + padding;

        const Rectangle shadow{ .x=badge_x + 2.0F, .y=badge_y + 2.0F, .width=badge_size, .height=badge_size };
        DrawRectangleRec(shadow, with_alpha(BLACK, 90));

        const Rectangle badge{ .x=badge_x, .y=badge_y, .width=badge_size, .height=badge_size };
        DrawRectangleRec(badge, Color{.r=220, .g=232, .b=246, .a=230});
        DrawRectangleLinesEx(badge, 1.0F, Color{.r=90, .g=120, .b=150, .a=220});
        DrawRectangleLinesEx(Rectangle{ .x=badge.x - 1.0F, .y=badge.y - 1.0F, .width=badge.width + 2.0F, .height=badge.height + 2.0F }, 1.0F, with_alpha(Color{.r=140, .g=180, .b=210, .a=255}, 90));

        const int left_eye_x = static_cast<int>(badge.x + (badge.width * 0.35F));
        const int right_eye_x = static_cast<int>(badge.x + (badge.width * 0.65F));
        const int eye_y = static_cast<int>(badge.y + (badge.height * 0.45F));
        const float eye_radius = badge.width * 0.08F;
        DrawCircle(left_eye_x, eye_y, eye_radius, Color{40, 60, 80, 255});
        DrawCircle(right_eye_x, eye_y, eye_radius, Color{40, 60, 80, 255});

        const int mouth_x = static_cast<int>(badge.x + (badge.width * 0.35F));
        const int mouth_y = static_cast<int>(badge.y + (badge.height * 0.65F));
        const int mouth_w = static_cast<int>(badge.width * 0.30F);
        DrawLine(mouth_x, mouth_y, mouth_x + mouth_w, mouth_y, Color{.r=40, .g=60, .b=80, .a=220});
    }
}

void CCardRenderer::on_update() {
    const auto config_opt = Query::try_find_first<GCardConfig>();
    if (!config_opt) {
        return;
    }

    const GCardConfig& config = config_opt->get();
    const auto ids = find_cards(CardAlias{});

    const float base_width = config.card_width > 1.0F ? config.card_width : 1.0F;
    const float base_height = config.card_height > 1.0F ? config.card_height : 1.0F;

    for (const EntityID entity_id : ids) {
        const CCard& card = Query::get<CCard>(entity_id);
        const CTransform& transform = Query::get<CTransform>(entity_id);

        const float scale_x = std::max(0.1F, transform.scale_x);
        const float scale_y = std::max(0.1F, transform.scale_y);
        const float scale = (scale_x + scale_y) * 0.5F;

        const float card_width = base_width * scale_x;
        const float card_height = base_height * scale_y;

        const float x = transform.position_x;
        const float y = transform.position_y;

        const Rectangle rect{ .x=x, .y=y, .width=card_width, .height=card_height };

        const float shadow_offset = std::max(2.0F, 4.0F * scale);
        DrawRectangle(
            static_cast<int>(rect.x + shadow_offset),
            static_cast<int>(rect.y + shadow_offset),
            static_cast<int>(rect.width),
            static_cast<int>(rect.height),
            with_alpha(BLACK, 85)
        );

        const Color face_up_color{.r=244, .g=239, .b=230, .a=255};
        const Color face_down_color{.r=36, .g=56, .b=78, .a=255};
        const Color matched_color{.r=156, .g=210, .b=170, .a=255};

        const Color fill_color = card.is_matched ? matched_color : (card.is_face_up ? face_up_color : face_down_color);
        const Color border_color = card.is_face_up ? Color{.r=100, .g=100, .b=110, .a=255} : Color{.r=120, .g=160, .b=200, .a=255};

        DrawRectangleRec(rect, fill_color);
        DrawRectangleLinesEx(rect, 2.0F, border_color);

        const float inset = std::max(3.0F, 5.0F * scale);
        const float highlight_height = std::max(6.0F, rect.height * 0.12F);
        const float shadow_height = std::max(6.0F, rect.height * 0.10F);

        if (rect.width > inset * 2.0F && rect.height > inset * 2.0F) {
            const Rectangle highlight{ .x=rect.x + inset, .y=rect.y + inset, .width=rect.width - (inset * 2.0F), .height=highlight_height };
            DrawRectangleRec(highlight, with_alpha(WHITE, card.is_face_up ? 70 : 35));

            const Rectangle shade{ .x=rect.x + inset, .y=rect.y + rect.height - shadow_height - inset, .width=rect.width - (inset * 2.0F), .height=shadow_height };
            DrawRectangleRec(shade, with_alpha(BLACK, card.is_face_up ? 30 : 55));
        }

        if (!card.is_face_up) {
            const float stripe_spacing = std::max(8.0F, 12.0F * scale);
            draw_stripes(rect, stripe_spacing, with_alpha(Color{80, 110, 140, 255}, 70));
            draw_mask_badge(rect, scale);
        }
    }
}
