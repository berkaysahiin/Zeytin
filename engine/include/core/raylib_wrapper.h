#pragma once

#include "raylib.h"
#include "raymath.h"

inline void init_window(int width, int height, const char* title) { return InitWindow(width, height, title); }
inline bool window_should_close() { return WindowShouldClose(); }
inline void close_window() { CloseWindow(); }
inline bool is_window_ready() { return IsWindowReady(); }
inline bool is_window_fullscreen() { return IsWindowFullscreen(); }
inline bool is_window_hidden() { return IsWindowHidden(); }
inline bool is_window_minimized() { return IsWindowMinimized(); }
inline bool is_window_maximized() { return IsWindowMaximized(); }
inline bool is_window_focused() { return IsWindowFocused(); }
inline bool is_window_resized() { return IsWindowResized(); }
inline void set_window_state(unsigned int flags) { SetWindowState(flags); }
inline void clear_window_state(unsigned int flags) { ClearWindowState(flags); }
inline void toggle_fullscreen() { ToggleFullscreen(); }
inline void maximize_window() { MaximizeWindow(); }
inline void minimize_window() { MinimizeWindow(); }
inline void restore_window() { RestoreWindow(); }
inline void set_window_title(const char* title) { SetWindowTitle(title); }
inline void set_window_position(int x, int y) { SetWindowPosition(x, y); }
inline void set_window_monitor(int monitor) { SetWindowMonitor(monitor); }
inline void set_window_min_size(int width, int height) { SetWindowMinSize(width, height); }
inline void set_window_size(int width, int height) { SetWindowSize(width, height); }
inline void set_window_opacity(float opacity) { SetWindowOpacity(opacity); }
inline void set_window_focused() { SetWindowFocused(); }
inline void set_target_fps(int fps) { SetTargetFPS(fps); }
inline int get_fps() { return GetFPS(); }
inline float get_frame_time() { return GetFrameTime(); }
inline double get_time() { return GetTime(); }

inline bool is_key_pressed(int key) { return IsKeyPressed(key); }
inline bool is_key_down(int key) { return IsKeyDown(key); }
inline bool is_key_released(int key) { return IsKeyReleased(key); }
inline bool is_key_up(int key) { return IsKeyUp(key); }
inline bool is_mouse_button_pressed(int button) { return IsMouseButtonPressed(button); }
inline bool is_mouse_button_down(int button) { return IsMouseButtonDown(button); }
inline bool is_mouse_button_released(int button) { return IsMouseButtonReleased(button); }
inline bool is_mouse_button_up(int button) { return IsMouseButtonUp(button); }
inline Vector2 get_mouse_position() { return GetMousePosition(); }
inline Vector2 get_mouse_delta() { return GetMouseDelta(); }
inline float get_mouse_wheel_move() { return GetMouseWheelMove(); }
inline void set_mouse_position(int x, int y) { SetMousePosition(x, y); }
inline void set_mouse_cursor(int cursor) { SetMouseCursor(cursor); }

inline void begin_drawing() { BeginDrawing(); }
inline void end_drawing() { EndDrawing(); }
inline void begin_mode2d(Camera2D camera) { BeginMode2D(camera); }
inline void end_mode2d() { EndMode2D(); }
inline void begin_texture_mode(RenderTexture2D target) { BeginTextureMode(target); }
inline void end_texture_mode() { EndTextureMode(); }
inline void clear_background(Color color) { ClearBackground(color); }
inline void draw_line(int startX, int startY, int endX, int endY, Color color) { DrawLine(startX, startY, endX, endY, color); }
inline void draw_line_v(Vector2 startPos, Vector2 endPos, Color color) { DrawLineV(startPos, endPos, color); }
inline void draw_circle(int centerX, int centerY, float radius, Color color) { DrawCircle(centerX, centerY, radius, color); }
inline void draw_circle_v(Vector2 center, float radius, Color color) { DrawCircleV(center, radius, color); }
inline void draw_circle_lines(int centerX, int centerY, float radius, Color color) { DrawCircleLines(centerX, centerY, radius, color); }
inline void draw_rectangle(int posX, int posY, int width, int height, Color color) { DrawRectangle(posX, posY, width, height, color); }
inline void draw_rectangle_v(Vector2 position, Vector2 size, Color color) { DrawRectangleV(position, size, color); }
inline void draw_rectangle_rec(Rectangle rec, Color color) { DrawRectangleRec(rec, color); }
inline void draw_rectangle_lines(int posX, int posY, int width, int height, Color color) { DrawRectangleLines(posX, posY, width, height, color); }
inline void draw_rectangle_lines_ex(Rectangle rec, float lineThick, Color color) { DrawRectangleLinesEx(rec, lineThick, color); }
inline void draw_text(const char* text, int posX, int posY, int fontSize, Color color) { DrawText(text, posX, posY, fontSize, color); }
inline void draw_texture(Texture2D texture, int posX, int posY, Color tint) { DrawTexture(texture, posX, posY, tint); }
inline void draw_texture_ex(Texture2D texture, Vector2 position, float rotation, float scale, Color tint) { DrawTextureEx(texture, position, rotation, scale, tint); }
inline void draw_texture_pro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint) { DrawTexturePro(texture, source, dest, origin, rotation, tint); }

inline Texture2D load_texture(const char* fileName) { return LoadTexture(fileName); }
inline void unload_texture(Texture2D texture) { UnloadTexture(texture); }
inline Image load_image(const char* fileName) { return LoadImage(fileName); }
inline void unload_image(Image image) { UnloadImage(image); }
inline Texture2D load_texture_from_image(Image image) { return LoadTextureFromImage(image); }
inline RenderTexture2D load_render_texture(int width, int height) { return LoadRenderTexture(width, height); }
inline void unload_render_texture(RenderTexture2D target) { UnloadRenderTexture(target); }

inline bool check_collision_recs(Rectangle rec1, Rectangle rec2) { return CheckCollisionRecs(rec1, rec2); }
inline bool check_collision_circles(Vector2 center1, float radius1, Vector2 center2, float radius2) { return CheckCollisionCircles(center1, radius1, center2, radius2); }
inline bool check_collision_circle_rec(Vector2 center, float radius, Rectangle rec) { return CheckCollisionCircleRec(center, radius, rec); }
inline bool check_collision_point_rec(Vector2 point, Rectangle rec) { return CheckCollisionPointRec(point, rec); }
inline bool check_collision_point_circle(Vector2 point, Vector2 center, float radius) { return CheckCollisionPointCircle(point, center, radius); }

inline Vector2 vector2_add(Vector2 v1, Vector2 v2) { return Vector2Add(v1, v2); }
inline Vector2 vector2_subtract(Vector2 v1, Vector2 v2) { return Vector2Subtract(v1, v2); }
inline float vector2_length(Vector2 v) { return Vector2Length(v); }
inline float vector2_distance_sqr(Vector2 v1, Vector2 v2) { return Vector2DistanceSqr(v1, v2); }
inline float vector2_distance(Vector2 v1, Vector2 v2) { return Vector2Distance(v1, v2); }
inline Vector2 vector2_scale(Vector2 v, float scale) { return Vector2Scale(v, scale); }
inline Vector2 vector2_normalize(Vector2 v) { return Vector2Normalize(v); }

inline Vector2 get_screen_to_world2d(Vector2 position, Camera2D camera) { return GetScreenToWorld2D(position, camera); }
inline Vector2 get_world_to_screen2d(Vector2 position, Camera2D camera) { return GetWorldToScreen2D(position, camera); }

inline bool file_exists(const char* fileName) { return FileExists(fileName); }
inline bool directory_exists(const char* dirPath) { return DirectoryExists(dirPath); }
inline const char* get_file_extension(const char* fileName) { return GetFileExtension(fileName); }
inline const char* get_file_name(const char* filePath) { return GetFileName(filePath); }

inline Color color_alpha(Color c, float alpha) { return ColorAlpha(c, alpha); }
inline Color color_tint(Color color, Color tint) { return ColorTint(color, tint); }
inline Color color_fade(Color color, float alpha) { return Fade(color, alpha); }

inline float get_random_value(int min, int max) { return GetRandomValue(min, max); }
inline void set_exit_key(int key) { SetExitKey(key); }
inline float get_screen_width() { return GetScreenWidth(); }
inline float get_screen_height() { return GetScreenHeight(); }

inline Color get_random_color() {
    return {
        static_cast<unsigned char>(GetRandomValue(0, 255)),
        static_cast<unsigned char>(GetRandomValue(0, 255)),  
        static_cast<unsigned char>(GetRandomValue(0, 255)), 
        255  
    };
}

