module;

#include "raylib.h"

export module zeytin.raylib;

export void init_window(int width, int height, const char* title);
export bool window_should_close();
export void close_window();
export bool is_window_ready();
export bool is_window_fullscreen();
export bool is_window_hidden();
export bool is_window_minimized();
export bool is_window_maximized();
export bool is_window_focused();
export bool is_window_resized();
export void set_window_state(unsigned int flags);
export void clear_window_state(unsigned int flags);
export void toggle_fullscreen();
export void maximize_window();
export void minimize_window();
export void restore_window();
export void set_window_title(const char* title);
export void set_window_position(int x, int y);
export void set_window_monitor(int monitor);
export void set_window_min_size(int width, int height);
export void set_window_size(int width, int height);
export void set_window_opacity(float opacity);
export void set_window_focused();
export void set_target_fps(int fps);
export int get_fps();
export float get_frame_time();
export double get_time();
export float get_screen_width();
export float get_screen_height();
export void set_exit_key(int key);

export bool is_key_pressed(int key);
export bool is_key_down(int key);
export bool is_key_released(int key);
export bool is_key_up(int key);

export bool is_mouse_button_pressed(int button);
export bool is_mouse_button_down(int button);
export bool is_mouse_button_released(int button);
export bool is_mouse_button_up(int button);
export Vector2 get_mouse_position();
export Vector2 get_mouse_delta();
export float get_mouse_wheel_move();
export void set_mouse_position(int x, int y);
export void set_mouse_cursor(int cursor);

export void begin_drawing();
export void end_drawing();
export void begin_mode2d(Camera2D camera);
export void end_mode2d();
export void begin_texture_mode(RenderTexture2D target);
export void end_texture_mode();
export void clear_background(Color color);

export void draw_line(int startX, int startY, int endX, int endY, Color color);
export void draw_line_v(Vector2 startPos, Vector2 endPos, Color color);
export void draw_line_ex(Vector2 startPos, Vector2 endPos, float thick, Color color);
export void draw_circle(int centerX, int centerY, float radius, Color color);
export void draw_circle_v(Vector2 center, float radius, Color color);
export void draw_circle_lines(int centerX, int centerY, float radius, Color color);
export void draw_rectangle(int posX, int posY, int width, int height, Color color);
export void draw_rectangle_v(Vector2 position, Vector2 size, Color color);
export void draw_rectangle_rec(Rectangle rec, Color color);
export void draw_rectangle_lines(int posX, int posY, int width, int height, Color color);
export void draw_rectangle_lines_ex(Rectangle rec, float lineThick, Color color);
export void draw_triangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color);

export void draw_text(const char* text, int posX, int posY, int fontSize, Color color);

export void draw_texture(Texture2D texture, int posX, int posY, Color tint);
export void draw_texture_ex(Texture2D texture, Vector2 position, float rotation, float scale, Color tint);
export void draw_texture_pro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint);
export Texture2D load_texture(const char* fileName);
export void unload_texture(Texture2D texture);
export Image load_image(const char* fileName);
export void unload_image(Image image);
export Texture2D load_texture_from_image(Image image);
export RenderTexture2D load_render_texture(int width, int height);
export void unload_render_texture(RenderTexture2D target);

export bool check_collision_recs(Rectangle rec1, Rectangle rec2);
export bool check_collision_circles(Vector2 center1, float radius1, Vector2 center2, float radius2);
export bool check_collision_circle_rec(Vector2 center, float radius, Rectangle rec);
export bool check_collision_point_rec(Vector2 point, Rectangle rec);
export bool check_collision_point_circle(Vector2 point, Vector2 center, float radius);

export Vector2 vector2_add(Vector2 v1, Vector2 v2);
export Vector2 vector2_subtract(Vector2 v1, Vector2 v2);
export float vector2_length(Vector2 v);
export float vector2_distance_sqr(Vector2 v1, Vector2 v2);
export float vector2_distance(Vector2 v1, Vector2 v2);
export Vector2 vector2_scale(Vector2 v, float scale);
export Vector2 vector2_normalize(Vector2 v);

export Vector2 get_screen_to_world2d(Vector2 position, Camera2D camera);
export Vector2 get_world_to_screen2d(Vector2 position, Camera2D camera);

export bool file_exists(const char* fileName);
export bool directory_exists(const char* dirPath);
export const char* get_file_extension(const char* fileName);
export const char* get_file_name(const char* filePath);

export Color color_alpha(Color c, float alpha);
export Color color_tint(Color color, Color tint);
export Color color_fade(Color color, float alpha);
export Color get_random_color();

export float get_random_value(int min, int max);
