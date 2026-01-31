module;

#include "raylib.h"
#include <algorithm>
#include <cmath>

module zeytin.game.background;
import zeytin.raylib;
import zeytin.zeytin;
import zeytin.resource;

void CBackground::on_early_update() {
	if (!enabled) {
		return;
	}

	const Camera2D& camera = Zeytin::get().get_camera();
	const Vector2 screen_top_left{.x=0.0F, .y=0.0F};
	const Vector2 screen_bottom_right{.x=VIRTUAL_WIDTH, .y=VIRTUAL_HEIGHT};
	const Vector2 world_top_left = get_screen_to_world2d(screen_top_left, camera);
	const Vector2 world_bottom_right = get_screen_to_world2d(screen_bottom_right, camera);

	const float min_x = std::min(world_top_left.x, world_bottom_right.x);
	const float min_y = std::min(world_top_left.y, world_bottom_right.y);
	const float width = std::max(1.0F, std::abs(world_bottom_right.x - world_top_left.x));
	const float height = std::max(1.0F, std::abs(world_bottom_right.y - world_top_left.y));

	ResourcePtr<Texture2D> background_texture(texture_path);
	if (!background_texture.is_valid()) {
		return;
	}
	Texture2D* tex = background_texture.get_ptr();
	const float tex_width = static_cast<float>(tex->width);
	const float tex_height = static_cast<float>(tex->height);
	const float view_aspect = width / height;
	const float tex_aspect = tex_width / tex_height;

	Rectangle source{
		.x=0.0F,
		.y=0.0F,
		.width=tex_width,
		.height=tex_height
	};

	if (view_aspect > tex_aspect) {
		const float crop_height = tex_width / view_aspect;
		source.y = (tex_height - crop_height) * 0.5F;
		source.height = crop_height;
	} else if (view_aspect < tex_aspect) {
		const float crop_width = tex_height * view_aspect;
		source.x = (tex_width - crop_width) * 0.5F;
		source.width = crop_width;
	}

	const float scale = 1.3F;
	const float scaled_width = width * scale;
	const float scaled_height = height * scale;
	const float offset_x = (scaled_width - width) * 0.5F;
	const float offset_y = (scaled_height - height) * 0.5F;
	const Rectangle dest{.x=min_x - offset_x, .y=min_y - offset_y, .width=scaled_width, .height=scaled_height};
	DrawTexturePro(*tex, source, dest, Vector2{.x=0, .y=0}, 0.0F, WHITE);
}
