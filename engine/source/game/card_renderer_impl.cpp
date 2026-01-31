module;
#include "raylib.h"
#include <algorithm>

module zeytin.game.card_renderer;
import zeytin.query;
import zeytin.logger;
import zeytin.game.card;
import zeytin.game.card_config;
import zeytin.game.transform;
import zeytin.game.aliases;
import zeytin.entity;

namespace {
    constexpr int k_mask_type_happy = 0;
    constexpr int k_mask_type_sad = 1;

    int pick_mask_type_random(const EntityID entity_id) {
        const unsigned long long base = static_cast<unsigned long long>(entity_id);
        const unsigned long long hashed = base * 11400714819323198485ULL;
        return (hashed & 1ULL) == 0ULL ? k_mask_type_happy : k_mask_type_sad;
    }

    template<typename... Ts>
    auto find_cards([[maybe_unused]] const Alias<Ts...>& unused) {
        static_assert(alias_contains_v<CCard, Ts...>, "Alias must include CCard");
        static_assert(alias_contains_v<CTransform, Ts...>, "Alias must include CTransform");
        return Query::find_all_with<Ts...>();
    }

    Color with_alpha(const Color base, const unsigned char alpha) {
        return Color{.r=base.r, .g=base.g, .b=base.b, .a=alpha};
    }

    float clamp01(const float value) {
        return std::clamp(value, 0.0F, 1.0F);
    }

    Color lerp_color(const Color left, const Color right, const float t) {
        const float amount = clamp01(t);
        return Color{
            .r=static_cast<unsigned char>(left.r + ((right.r - left.r) * amount)),
            .g=static_cast<unsigned char>(left.g + ((right.g - left.g) * amount)),
            .b=static_cast<unsigned char>(left.b + ((right.b - left.b) * amount)),
            .a=static_cast<unsigned char>(left.a + ((right.a - left.a) * amount))
        };
    }

    Color darken_color(const Color base, const float amount) {
        const float strength = clamp01(amount);
        return Color{
            .r=static_cast<unsigned char>(base.r * (1.0F - strength)),
            .g=static_cast<unsigned char>(base.g * (1.0F - strength)),
            .b=static_cast<unsigned char>(base.b * (1.0F - strength)),
            .a=base.a
        };
    }

    Color lighten_color(const Color base, const float amount) {
        const float strength = clamp01(amount);
        return Color{
            .r=static_cast<unsigned char>(base.r + ((255 - base.r) * strength)),
            .g=static_cast<unsigned char>(base.g + ((255 - base.g) * strength)),
            .b=static_cast<unsigned char>(base.b + ((255 - base.b) * strength)),
            .a=base.a
        };
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

    void draw_leather_texture(const Rectangle rect, const float scale, const Color base_color, const float texture_strength) {
        const float grain_spacing = std::max(3.0F, 5.5F * scale);
        const float pore_radius = std::max(0.6F, 1.0F * scale);
        const float blotch_radius = std::max(1.6F, 2.6F * scale);

        const float strength = clamp01(texture_strength);
        const Color pore_dark = darken_color(base_color, 0.25F);
        const Color pore_light = lighten_color(base_color, 0.18F);
        const Color blotch_color = darken_color(base_color, 0.18F);
        const Color blotch_highlight = lighten_color(base_color, 0.22F);

        const int start_x = static_cast<int>(rect.x + 2.0F);
        const int end_x = static_cast<int>(rect.x + rect.width - 2.0F);
        const int start_y = static_cast<int>(rect.y + 2.0F);
        const int end_y = static_cast<int>(rect.y + rect.height - 2.0F);

        int row_index = 0;
        for (int pos_y = start_y; pos_y < end_y; pos_y += static_cast<int>(grain_spacing)) {
            const int offset = (row_index % 2 == 0) ? 0 : static_cast<int>(grain_spacing * 0.5F);
            for (int pos_x = start_x + offset; pos_x < end_x; pos_x += static_cast<int>(grain_spacing * 1.25F)) {
                const int hash = (pos_x * 37 + pos_y * 57) % 13;
                const bool alternate = (hash % 2) == 0;
                const unsigned char pore_alpha = static_cast<unsigned char>(std::clamp(70.0F * strength, 0.0F, 90.0F));
                const unsigned char pore_light_alpha = static_cast<unsigned char>(std::clamp(45.0F * strength, 0.0F, 70.0F));
                const Color pore_color = alternate ? with_alpha(pore_dark, pore_alpha)
                                                   : with_alpha(pore_light, pore_light_alpha);
                if (pore_alpha > 0 || pore_light_alpha > 0) {
                    DrawCircle(pos_x, pos_y, pore_radius, pore_color);
                }

                if (hash == 0 || hash == 5) {
                    const unsigned char blotch_alpha = static_cast<unsigned char>(std::clamp(35.0F * strength, 0.0F, 60.0F));
                    if (blotch_alpha > 0) {
                        DrawCircle(pos_x + 1, pos_y + 1, blotch_radius, with_alpha(blotch_color, blotch_alpha));
                    }
                }
                if (hash == 7) {
                    const unsigned char highlight_alpha = static_cast<unsigned char>(std::clamp(22.0F * strength, 0.0F, 45.0F));
                    if (highlight_alpha > 0) {
                        DrawCircle(pos_x - 1, pos_y - 1, blotch_radius * 0.8F, with_alpha(blotch_highlight, highlight_alpha));
                    }
                }
            }
            row_index++;
        }
    }

    void draw_single_mask(const float center_x, const float center_y, const float mask_width, const float mask_height, 
                          const bool is_happy, const float scale, const float mask_opacity, const float rotation) {
        
        const unsigned char base_alpha = static_cast<unsigned char>(std::clamp(245.0F * clamp01(mask_opacity), 0.0F, 255.0F));
        const unsigned char outline_alpha = static_cast<unsigned char>(std::clamp(230.0F * clamp01(mask_opacity), 0.0F, 255.0F));
        const Color mask_base{.r=240, .g=235, .b=225, .a=base_alpha};
        const Color mask_outline{.r=40, .g=40, .b=40, .a=outline_alpha};

        const float shadow_offset = std::max(1.5F, 2.0F * scale);

        DrawEllipse(
            static_cast<int>(center_x + shadow_offset),
            static_cast<int>(center_y + shadow_offset),
            mask_width * 0.5F,
            mask_height * 0.5F,
            with_alpha(BLACK, 85)
        );

        DrawEllipse(
            static_cast<int>(center_x), 
            static_cast<int>(center_y), 
            mask_width * 0.5F, 
            mask_height * 0.5F, 
            mask_base
        );

        DrawEllipseLines(
            static_cast<int>(center_x), 
            static_cast<int>(center_y), 
            mask_width * 0.5F, 
            mask_height * 0.5F, 
            mask_outline
        );

        const bool sad_emotion = !is_happy;
        const int eye_left_x = static_cast<int>(center_x - (mask_width * 0.22F));
        const int eye_right_x = static_cast<int>(center_x + (mask_width * 0.22F));
        const int eye_pos_y = static_cast<int>(center_y - (mask_height * (sad_emotion ? 0.08F : 0.12F)));
        const float eye_rx = mask_width * (sad_emotion ? 0.10F : 0.12F);
        const float eye_ry = mask_height * (sad_emotion ? 0.07F : 0.10F);

        DrawEllipse(eye_left_x, eye_pos_y, eye_rx, eye_ry, with_alpha(BLACK, outline_alpha));
        DrawEllipse(eye_right_x, eye_pos_y, eye_rx, eye_ry, with_alpha(BLACK, outline_alpha));

        const float brow_thickness = std::max(1.6F, 2.2F * scale);
        const int brow_y_offset = static_cast<int>(eye_ry + (mask_height * 0.07F));
        
        if (is_happy) {
            DrawLineEx(
                Vector2{.x=static_cast<float>(eye_left_x - eye_rx), .y=static_cast<float>(eye_pos_y - brow_y_offset)},
                Vector2{.x=static_cast<float>(eye_left_x + eye_rx), .y=static_cast<float>(eye_pos_y - brow_y_offset - 5)},
                brow_thickness,
                mask_outline
            );
            DrawLineEx(
                Vector2{.x=static_cast<float>(eye_right_x - eye_rx), .y=static_cast<float>(eye_pos_y - brow_y_offset - 5)},
                Vector2{.x=static_cast<float>(eye_right_x + eye_rx), .y=static_cast<float>(eye_pos_y - brow_y_offset)},
                brow_thickness,
                mask_outline
            );
        } else {
            DrawLineEx(
                Vector2{.x=static_cast<float>(eye_left_x - eye_rx), .y=static_cast<float>(eye_pos_y - brow_y_offset - 6)},
                Vector2{.x=static_cast<float>(eye_left_x + eye_rx), .y=static_cast<float>(eye_pos_y - brow_y_offset)},
                brow_thickness,
                mask_outline
            );
            DrawLineEx(
                Vector2{.x=static_cast<float>(eye_right_x - eye_rx), .y=static_cast<float>(eye_pos_y - brow_y_offset)},
                Vector2{.x=static_cast<float>(eye_right_x + eye_rx), .y=static_cast<float>(eye_pos_y - brow_y_offset - 6)},
                brow_thickness,
                mask_outline
            );
        }

        const int mouth_y = static_cast<int>(center_y + (mask_height * (sad_emotion ? 0.22F : 0.16F)));
        const float mouth_width = mask_width * (sad_emotion ? 0.34F : 0.38F);
        const float mouth_curve = mask_height * (sad_emotion ? 0.16F : 0.18F);
        const float mouth_thickness = std::max(2.2F, 2.8F * scale);

        const int segments = 10;
        for (int i = 0; i < segments; i++) {
            const float t1 = static_cast<float>(i) / segments;
            const float t2 = static_cast<float>(i + 1) / segments;
            
            const float x1 = center_x - mouth_width + (t1 * mouth_width * 2.0F);
            const float x2 = center_x - mouth_width + (t2 * mouth_width * 2.0F);
            
            const float curve_factor1 = 1.0F - ((2.0F * t1 - 1.0F) * (2.0F * t1 - 1.0F));
            const float curve_factor2 = 1.0F - ((2.0F * t2 - 1.0F) * (2.0F * t2 - 1.0F));
            
            const float y1 = is_happy ? (mouth_y - (mouth_curve * curve_factor1)) : (mouth_y + (mouth_curve * curve_factor1));
            const float y2 = is_happy ? (mouth_y - (mouth_curve * curve_factor2)) : (mouth_y + (mouth_curve * curve_factor2));
            
            DrawLineEx(
                Vector2{.x=x1, .y=y1},
                Vector2{.x=x2, .y=y2},
                mouth_thickness,
                mask_outline
            );
        }
    }

    void draw_mask_badge(const Rectangle card_rect, const float scale, const float mask_scale, const float mask_opacity, const int mask_type) {
        const float safe_scale = std::max(0.2F, mask_scale);
        const float mask_size = std::min(card_rect.width, card_rect.height) * safe_scale;
        if (mask_size < 10.0F) {
            return;
        }

        const float center_x = card_rect.x + (card_rect.width * 0.5F);
        const float center_y = card_rect.y + (card_rect.height * 0.5F);

        const float mask_width = mask_size * 0.80F;
        const float mask_height = mask_size * 0.88F;
        const bool is_happy = mask_type != k_mask_type_sad;
        const float rotation_angle = is_happy ? -6.0F : 6.0F;
        draw_single_mask(center_x, center_y, mask_width, mask_height, is_happy, scale, mask_opacity, rotation_angle);
    }
}

void CCardRenderer::on_update() {
    const auto config_opt = Query::try_find_first<GCardConfig>();
    if (!config_opt) {
        static bool logged_missing_config = false;
        if (!logged_missing_config) {
            log_error("Card renderer missing card config");
            logged_missing_config = true;
        }
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
        const float back_tone = clamp01(config.back_leather_tone);
        const Color back_dark{.r=72, .g=46, .b=32, .a=255};
        const Color back_light{.r=128, .g=90, .b=62, .a=255};
        const Color face_down_color = lerp_color(back_dark, back_light, back_tone);
        const Color matched_color{.r=156, .g=210, .b=170, .a=255};

        const Color fill_color = card.is_matched ? matched_color : (card.is_face_up ? face_up_color : face_down_color);
        const Color border_color = card.is_face_up ? Color{.r=100, .g=100, .b=110, .a=255} : darken_color(face_down_color, 0.35F);

        DrawRectangleRec(rect, fill_color);
        DrawRectangleLinesEx(rect, 2.0F, border_color);

        const float inset = std::max(3.0F, 5.0F * scale);
        const float highlight_height = std::max(6.0F, rect.height * 0.12F);
        const float shadow_height = std::max(6.0F, rect.height * 0.10F);

        if (rect.width > inset * 2.0F && rect.height > inset * 2.0F) {
            const float back_highlight_strength = clamp01(config.back_highlight_strength);
            const unsigned char back_highlight_alpha = static_cast<unsigned char>(std::clamp(10.0F + (80.0F * back_highlight_strength), 0.0F, 120.0F));
            const Rectangle highlight{ .x=rect.x + inset, .y=rect.y + inset, .width=rect.width - (inset * 2.0F), .height=highlight_height };
            DrawRectangleRec(highlight, with_alpha(WHITE, card.is_face_up ? 70 : back_highlight_alpha));

            const Rectangle shade{ .x=rect.x + inset, .y=rect.y + rect.height - shadow_height - inset, .width=rect.width - (inset * 2.0F), .height=shadow_height };
            const float back_shadow_strength = clamp01(config.back_shadow_strength);
            const unsigned char back_shadow_alpha = static_cast<unsigned char>(std::clamp(16.0F + (90.0F * back_shadow_strength), 0.0F, 140.0F));
            DrawRectangleRec(shade, with_alpha(BLACK, card.is_face_up ? 30 : back_shadow_alpha));
        }

        if (!card.is_face_up) {
            const float inner_inset = std::max(4.0F, 7.0F * scale);
            const Rectangle inner_rect{ .x=rect.x + inner_inset, .y=rect.y + inner_inset, .width=rect.width - (inner_inset * 2.0F), .height=rect.height - (inner_inset * 2.0F) };
            const Color inner_highlight = lighten_color(face_down_color, 0.25F);
            const Color inner_shadow = darken_color(face_down_color, 0.35F);
            DrawRectangleLinesEx(inner_rect, 1.0F, with_alpha(inner_highlight, 70));
            DrawRectangleLinesEx(Rectangle{ .x=inner_rect.x + 1.0F, .y=inner_rect.y + 1.0F, .width=inner_rect.width - 2.0F, .height=inner_rect.height - 2.0F }, 1.0F, with_alpha(inner_shadow, 100));

            draw_leather_texture(rect, scale, face_down_color, config.back_texture_strength);
            const int mask_type = pick_mask_type_random(entity_id);
            draw_mask_badge(rect, scale, config.mask_scale, config.mask_opacity, mask_type);
        }
    }
}
