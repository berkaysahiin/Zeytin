module;

#include "raylib.h"
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

		if (card.symbol_id == 0) {
            ResourcePtr<Texture2D> symbol_texture(symbol_0_path);
            if (symbol_texture.is_valid()) {
                Texture2D* tex = symbol_texture.get_ptr();
                const Rectangle source{.x=0, .y=0, .width=static_cast<float>(tex->width), .height=static_cast<float>(tex->height)};
                const Rectangle dest{.x=face_center.x - half_width, .y=face_center.y - (half_height / 1.2F), .width=width, .height=height};
                DrawTexturePro(*tex, source, dest, Vector2{.x=0, .y=0}, 0.0F, WHITE);
            } else {
                log_error("Couldn't find valid resource: {}", symbol_0_path);
			}
        }

  		if (card.symbol_id == 1) {
            ResourcePtr<Texture2D> symbol_texture(symbol_1_path);
            if (symbol_texture.is_valid()) {
                Texture2D* tex = symbol_texture.get_ptr();
                const Rectangle source{.x=0, .y=0, .width=static_cast<float>(tex->width), .height=static_cast<float>(tex->height)};
                const Rectangle dest{.x=face_center.x - half_width, .y=face_center.y - (half_height / 1.2F), .width=width, .height=height};
                DrawTexturePro(*tex, source, dest, Vector2{.x=0, .y=0}, 0.0F, WHITE);
            } else {
                log_error("Couldn't find valid resource: {}", symbol_1_path);
            }
        } 

        if (card.symbol_id == 2) {
            ResourcePtr<Texture2D> symbol_texture(symbol_2_path);
            if (symbol_texture.is_valid()) {
                Texture2D* tex = symbol_texture.get_ptr();
                const Rectangle source{.x=0, .y=0, .width=static_cast<float>(tex->width), .height=static_cast<float>(tex->height)};
                const Rectangle dest{.x=face_center.x - half_width, .y=face_center.y - (half_height / 1.2F), .width=width, .height=height};
                DrawTexturePro(*tex, source, dest, Vector2{.x=0, .y=0}, 0.0F, WHITE);
            } else {
                log_error("Couldn't find valid resource: {}", symbol_2_path);
            }
        } 

		if (card.symbol_id == 3) {
            ResourcePtr<Texture2D> symbol_texture(symbol_3_path);
            if (symbol_texture.is_valid()) {
                Texture2D* tex = symbol_texture.get_ptr();
                const Rectangle source{.x=0, .y=0, .width=static_cast<float>(tex->width), .height=static_cast<float>(tex->height)};
                const Rectangle dest{.x=face_center.x - half_width, .y=face_center.y - (half_height / 1.2F), .width=width, .height=height};
                DrawTexturePro(*tex, source, dest, Vector2{.x=0, .y=0}, 0.0F, WHITE);
            } else {
                log_error("Couldn't find valid resource: {}", symbol_3_path);
            }
        } 
		if (card.symbol_id == 4) {
            ResourcePtr<Texture2D> symbol_texture(symbol_4_path);
            if (symbol_texture.is_valid()) {
                Texture2D* tex = symbol_texture.get_ptr();
                const Rectangle source{.x=0, .y=0, .width=static_cast<float>(tex->width), .height=static_cast<float>(tex->height)};
                const Rectangle dest{.x=face_center.x - half_width, .y=face_center.y - (half_height / 1.2F), .width=width, .height=height};
                DrawTexturePro(*tex, source, dest, Vector2{.x=0, .y=0}, 0.0F, WHITE);
            } else {
                log_error("Couldn't find valid resource: {}", symbol_4_path);
            }
        } 

        return;
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
