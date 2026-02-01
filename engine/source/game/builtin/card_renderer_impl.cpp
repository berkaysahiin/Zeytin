module;

#include "raylib.h"
#include <cmath>
#include <string>

module zeytin.game.card_renderer;
import zeytin.raylib;
import zeytin.game.transform;
import zeytin.game.collider;
import zeytin.game.card;
import zeytin.game.card_config;
import zeytin.query;
import zeytin.resource;
import zeytin.logger;

namespace {
    const std::string& get_symbol_path(const CCardRenderer& renderer, const int symbol_id) {
        switch (symbol_id) {
            case 0: return renderer.symbol_0_path;
            case 1: return renderer.symbol_1_path;
            case 2: return renderer.symbol_2_path;
            case 3: return renderer.symbol_3_path;
            case 4: return renderer.symbol_4_path;
            case 5: return renderer.symbol_5_path;
            case 6: return renderer.symbol_6_path;
            case 7: return renderer.symbol_7_path;
            default: return renderer.symbol_0_path;
        }
    }

    void draw_face_texture(
        const std::string& texture_path,
        const Vector2 center,
        const float width,
        const float height,
        const float scale_x
    ) {
        ResourcePtr<Texture2D> texture(texture_path);
        if (!texture.is_valid()) {
            log_error("Couldn't find valid resource: {}", texture_path);
            return;
        }

        Texture2D* tex = texture.get_ptr();
        const Rectangle source{.x=0, .y=0, .width=static_cast<float>(tex->width), .height=static_cast<float>(tex->height)};
        const float scaled_width = width * scale_x;
        const Rectangle dest{
            .x=center.x - scaled_width * 0.5F,
            .y=center.y - (height * 0.5F),
            .width=scaled_width,
            .height=height
        };
        DrawTexturePro(*tex, source, dest, Vector2{.x=0, .y=0}, 0.0F, WHITE);
    }

    void draw_card_face(
        const CCardRenderer& renderer,
        const CCard& card,
        const CardMaskStatus mask_status,
        const Vector2 center,
        const float width,
        const float height,
        const float scale_x
    ) {
        if (mask_status == CardMaskStatus::NO_MASK) {
            draw_face_texture(get_symbol_path(renderer, card.symbol_id), center, width, height, scale_x);
        } else if (mask_status == CardMaskStatus::SMILE) {
            draw_face_texture(renderer.mask_happy_path, center, width, height, scale_x);
        } else {
            draw_face_texture(renderer.mask_sad_path, center, width, height, scale_x);
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
    if (!m_initialized) {
        m_last_mask_status = mask_status;
        m_initialized = true;
    }

    if (mask_status != m_last_mask_status) {
        m_flip_from = m_last_mask_status;
        m_flip_to = mask_status;
        m_last_mask_status = mask_status;
        m_flip_timer = 0.0F;
        m_flipping = true;
    }

    float flip_duration = 0.2F;
    const auto config_opt = Query::try_find_first<GCardConfig>();
    if (config_opt) {
        flip_duration = config_opt->get().flip_duration;
    }

    if (m_flipping && flip_duration > 0.0F) {
        m_flip_timer += get_frame_time();
        const float t = m_flip_timer / flip_duration;
        const float clamped = t > 1.0F ? 1.0F : t;
        const float scale_x = std::fabs(cosf(clamped * 3.1415926F));
        const CardMaskStatus face = clamped < 0.5F ? m_flip_from : m_flip_to;
        draw_card_face(*this, card, face, face_center, width, height, scale_x);
        if (clamped >= 1.0F) {
            m_flipping = false;
            m_flip_timer = 0.0F;
        }
    } else {
        draw_card_face(*this, card, mask_status, face_center, width, height, 1.0F);
    }
}
