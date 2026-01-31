module;

#include "raylib.h"
#include <algorithm>
#include <cmath>

module zeytin.game.background;
import zeytin.raylib;
import zeytin.zeytin;

namespace {
	int clamp_channel(const int value) {
		return std::clamp(value, 0, 255);
	}

	Color make_color(const int red, const int green, const int blue, const int alpha) {
		return Color{
			.r=static_cast<unsigned char>(clamp_channel(red)),
			.g=static_cast<unsigned char>(clamp_channel(green)),
			.b=static_cast<unsigned char>(clamp_channel(blue)),
			.a=static_cast<unsigned char>(clamp_channel(alpha))
		};
	}

	Color lerp_color(const Color left, const Color right, const float amount) {
		const float t = std::clamp(amount, 0.0F, 1.0F);
		return Color{
			.r=static_cast<unsigned char>(left.r + ((right.r - left.r) * t)),
			.g=static_cast<unsigned char>(left.g + ((right.g - left.g) * t)),
			.b=static_cast<unsigned char>(left.b + ((right.b - left.b) * t)),
			.a=static_cast<unsigned char>(left.a + ((right.a - left.a) * t))
		};
	}
}

void CBackground::on_early_update() {
	if (!enabled) {
		return;
	}

	const Camera2D& camera = Zeytin::get().get_camera();
	const Vector2 screen_top_left{.x=0.0F, .y=0.0F};
	const Vector2 screen_bottom_right{.x=get_screen_width(), .y=get_screen_height()};
	const Vector2 world_top_left = get_screen_to_world2d(screen_top_left, camera);
	const Vector2 world_bottom_right = get_screen_to_world2d(screen_bottom_right, camera);

	const float min_x = std::min(world_top_left.x, world_bottom_right.x);
	const float min_y = std::min(world_top_left.y, world_bottom_right.y);
	const float width = std::max(1.0F, std::abs(world_bottom_right.x - world_top_left.x));
	const float height = std::max(1.0F, std::abs(world_bottom_right.y - world_top_left.y));

	const Color base_color = make_color(base_red, base_green, base_blue, base_alpha);
	const Color accent_color = make_color(accent_red, accent_green, accent_blue, accent_alpha);

	DrawRectangleRec(Rectangle{.x=min_x, .y=min_y, .width=width, .height=height}, base_color);

	const float strength = std::clamp(motion_strength, 0.0F, 1.0F);
	if (strength <= 0.001F || accent_color.a == 0) {
		return;
	}

	const float time = get_time() * motion_speed;
	const float blob_base = std::max(width, height) * std::max(0.2F, blob_scale);
	const float offset_x = width * strength;
	const float offset_y = height * strength;
	const float center_x = min_x + (width * 0.5F);
	const float center_y = min_y + (height * 0.5F);

	for (int index = 0; index < 3; ++index) {
		const float phase = static_cast<float>(index) * 2.1F;
		const float drift_x = std::sin(time + phase) * offset_x;
		const float drift_y = std::cos(time * 0.9F + phase) * offset_y;
		const float secondary_x = std::cos(time * 0.6F + phase) * offset_x * 0.35F;
		const float secondary_y = std::sin(time * 0.7F + phase) * offset_y * 0.35F;

		const Vector2 center{
			.x=center_x + drift_x + secondary_x,
			.y=center_y + drift_y + secondary_y
		};

		const float radius = blob_base * (0.9F + (0.18F * index));
		const float tint = 0.2F + (0.3F * index);
		const Color blob_color = lerp_color(accent_color, base_color, tint);
		DrawCircleV(center, radius, blob_color);
	}
}
