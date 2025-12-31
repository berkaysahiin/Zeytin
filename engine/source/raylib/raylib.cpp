module;

#include "raylib.h"
#include "raymath.h"

module zeytin.raylib;

void init_window(int width, int height, const char* title) { InitWindow(width, height, title); }
bool window_should_close() { return WindowShouldClose(); }
void close_window() { CloseWindow(); }
bool is_window_ready() { return IsWindowReady(); }
bool is_window_fullscreen() { return IsWindowFullscreen(); }
bool is_window_hidden() { return IsWindowHidden(); }
bool is_window_minimized() { return IsWindowMinimized(); }
bool is_window_maximized() { return IsWindowMaximized(); }
bool is_window_focused() { return IsWindowFocused(); }
bool is_window_resized() { return IsWindowResized(); }
void set_window_state(unsigned int flags) { SetWindowState(flags); }
void clear_window_state(unsigned int flags) { ClearWindowState(flags); }
void toggle_fullscreen() { ToggleFullscreen(); }
void maximize_window() { MaximizeWindow(); }
void minimize_window() { MinimizeWindow(); }
void restore_window() { RestoreWindow(); }
void set_window_title(const char* title) { SetWindowTitle(title); }
void set_window_position(int x, int y) { SetWindowPosition(x, y); }
void set_window_monitor(int monitor) { SetWindowMonitor(monitor); }
void set_window_min_size(int width, int height) { SetWindowMinSize(width, height); }
void set_window_size(int width, int height) { SetWindowSize(width, height); }
void set_window_opacity(float opacity) { SetWindowOpacity(opacity); }
void set_window_focused() { SetWindowFocused(); }
void set_target_fps(int fps) { SetTargetFPS(fps); }
int get_fps() { return GetFPS(); }
float get_frame_time() { return GetFrameTime(); }
double get_time() { return GetTime(); }
float get_screen_width() { return GetScreenWidth(); }
float get_screen_height() { return GetScreenHeight(); }
void set_exit_key(int key) { SetExitKey(key); }

bool is_key_pressed(int key) { return IsKeyPressed(key); }
bool is_key_down(int key) { return IsKeyDown(key); }
bool is_key_released(int key) { return IsKeyReleased(key); }
bool is_key_up(int key) { return IsKeyUp(key); }

bool is_mouse_button_pressed(int button) { return IsMouseButtonPressed(button); }
bool is_mouse_button_down(int button) { return IsMouseButtonDown(button); }
bool is_mouse_button_released(int button) { return IsMouseButtonReleased(button); }
bool is_mouse_button_up(int button) { return IsMouseButtonUp(button); }
Vector2 get_mouse_position() { return GetMousePosition(); }
Vector2 get_mouse_delta() { return GetMouseDelta(); }
float get_mouse_wheel_move() { return GetMouseWheelMove(); }
void set_mouse_position(int x, int y) { SetMousePosition(x, y); }
void set_mouse_cursor(int cursor) { SetMouseCursor(cursor); }

void begin_drawing() { BeginDrawing(); }
void end_drawing() { EndDrawing(); }
void begin_mode2d(Camera2D camera) { BeginMode2D(camera); }
void end_mode2d() { EndMode2D(); }
void begin_texture_mode(RenderTexture2D target) { BeginTextureMode(target); }
void end_texture_mode() { EndTextureMode(); }
void clear_background(Color color) { ClearBackground(color); }

void draw_line(int startX, int startY, int endX, int endY, Color color) { DrawLine(startX, startY, endX, endY, color); }
void draw_line_v(Vector2 startPos, Vector2 endPos, Color color) { DrawLineV(startPos, endPos, color); }
void draw_circle(int centerX, int centerY, float radius, Color color) { DrawCircle(centerX, centerY, radius, color); }
void draw_circle_v(Vector2 center, float radius, Color color) { DrawCircleV(center, radius, color); }
void draw_circle_lines(int centerX, int centerY, float radius, Color color) { DrawCircleLines(centerX, centerY, radius, color); }
void draw_rectangle(int posX, int posY, int width, int height, Color color) { DrawRectangle(posX, posY, width, height, color); }
void draw_rectangle_v(Vector2 position, Vector2 size, Color color) { DrawRectangleV(position, size, color); }
void draw_rectangle_rec(Rectangle rec, Color color) { DrawRectangleRec(rec, color); }
void draw_rectangle_lines(int posX, int posY, int width, int height, Color color) { DrawRectangleLines(posX, posY, width, height, color); }
void draw_rectangle_lines_ex(Rectangle rec, float lineThick, Color color) { DrawRectangleLinesEx(rec, lineThick, color); }
void draw_triangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color) { DrawTriangle(v1, v2, v3, color); }

void draw_text(const char* text, int posX, int posY, int fontSize, Color color) { DrawText(text, posX, posY, fontSize, color); }

void draw_texture(Texture2D texture, int posX, int posY, Color tint) { DrawTexture(texture, posX, posY, tint); }
void draw_texture_ex(Texture2D texture, Vector2 position, float rotation, float scale, Color tint) { DrawTextureEx(texture, position, rotation, scale, tint); }
void draw_texture_pro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint) { DrawTexturePro(texture, source, dest, origin, rotation, tint); }
Texture2D load_texture(const char* fileName) { return LoadTexture(fileName); }
void unload_texture(Texture2D texture) { UnloadTexture(texture); }
Image load_image(const char* fileName) { return LoadImage(fileName); }
void unload_image(Image image) { UnloadImage(image); }
Texture2D load_texture_from_image(Image image) { return LoadTextureFromImage(image); }
RenderTexture2D load_render_texture(int width, int height) { return LoadRenderTexture(width, height); }
void unload_render_texture(RenderTexture2D target) { UnloadRenderTexture(target); }

bool check_collision_recs(Rectangle rec1, Rectangle rec2) { return CheckCollisionRecs(rec1, rec2); }
bool check_collision_circles(Vector2 center1, float radius1, Vector2 center2, float radius2) { return CheckCollisionCircles(center1, radius1, center2, radius2); }
bool check_collision_circle_rec(Vector2 center, float radius, Rectangle rec) { return CheckCollisionCircleRec(center, radius, rec); }
bool check_collision_point_rec(Vector2 point, Rectangle rec) { return CheckCollisionPointRec(point, rec); }
bool check_collision_point_circle(Vector2 point, Vector2 center, float radius) { return CheckCollisionPointCircle(point, center, radius); }

Vector2 vector2_add(Vector2 v1, Vector2 v2) { return Vector2Add(v1, v2); }
Vector2 vector2_subtract(Vector2 v1, Vector2 v2) { return Vector2Subtract(v1, v2); }
float vector2_length(Vector2 v) { return Vector2Length(v); }
float vector2_distance_sqr(Vector2 v1, Vector2 v2) { return Vector2DistanceSqr(v1, v2); }
float vector2_distance(Vector2 v1, Vector2 v2) { return Vector2Distance(v1, v2); }
Vector2 vector2_scale(Vector2 v, float scale) { return Vector2Scale(v, scale); }
Vector2 vector2_normalize(Vector2 v) { return Vector2Normalize(v); }

Vector2 get_screen_to_world2d(Vector2 position, Camera2D camera) { return GetScreenToWorld2D(position, camera); }
Vector2 get_world_to_screen2d(Vector2 position, Camera2D camera) { return GetWorldToScreen2D(position, camera); }

bool file_exists(const char* fileName) { return FileExists(fileName); }
bool directory_exists(const char* dirPath) { return DirectoryExists(dirPath); }
const char* get_file_extension(const char* fileName) { return GetFileExtension(fileName); }
const char* get_file_name(const char* filePath) { return GetFileName(filePath); }

Color color_alpha(Color c, float alpha) { return ColorAlpha(c, alpha); }
Color color_tint(Color color, Color tint) { return ColorTint(color, tint); }
Color color_fade(Color color, float alpha) { return Fade(color, alpha); }
Color get_random_color() {
    return {
        static_cast<unsigned char>(GetRandomValue(0, 255)),
        static_cast<unsigned char>(GetRandomValue(0, 255)),
        static_cast<unsigned char>(GetRandomValue(0, 255)),
        255
    };
}

float get_random_value(int min, int max) { return GetRandomValue(min, max); }
