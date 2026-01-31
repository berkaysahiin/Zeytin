module;

#include "raylib.h"

module zeytin.game.crt_effect;
import zeytin.logger;
import zeytin.raylib;

const char* CRTEffect::get_fragment_shader_code() const {
    return R"(
#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform float time;
uniform vec2 resolution;

uniform float curvature;
uniform float scanlineIntensity;
uniform float vignetteIntensity;
uniform float noiseIntensity;
uniform float chromaticAberration;

float noise(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec2 curveRemapUV(vec2 uv) {
    uv = uv * 2.0 - 1.0;
    vec2 offset = abs(uv.yx) / vec2(curvature, curvature);
    uv = uv + uv * offset * offset;
    uv = uv * 0.5 + 0.5;
    return uv;
}

float vignette(vec2 uv) {
    uv *= 1.0 - uv.yx;
    float vig = uv.x * uv.y * 16.0;
    return pow(vig, vignetteIntensity);
}

float scanline(vec2 uv) {
    float line = sin(uv.y * resolution.y * 1.25) * 0.5 + 0.5;
    float mask = sin(uv.x * resolution.x * 0.25) * 0.5 + 0.5;
    return mix(line, line * mask, 0.35);
}

vec3 soft_blur(vec2 uv) {
    vec2 texel = 1.0 / resolution;
    vec3 sum = texture(texture0, uv).rgb * 0.36;
    sum += texture(texture0, uv + vec2(texel.x, 0.0)).rgb * 0.16;
    sum += texture(texture0, uv - vec2(texel.x, 0.0)).rgb * 0.16;
    sum += texture(texture0, uv + vec2(0.0, texel.y)).rgb * 0.16;
    sum += texture(texture0, uv - vec2(0.0, texel.y)).rgb * 0.16;
    return sum;
}

vec3 tone_curve(vec3 color) {
    color = pow(color, vec3(0.96));
    color = mix(color, color * color * (3.0 - 2.0 * color), 0.25);
    color *= vec3(1.02, 1.0, 0.98);
    return color;
}

void main() {
    vec2 uv = fragTexCoord;

    if (curvature > 0.0) {
        uv = curveRemapUV(uv);
    }

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        finalColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec3 color;
    if (chromaticAberration > 0.0) {
        float offset = chromaticAberration * 0.006;
        color.r = texture(texture0, vec2(uv.x + offset, uv.y)).r;
        color.g = texture(texture0, uv).g;
        color.b = texture(texture0, vec2(uv.x - offset, uv.y)).b;
    } else {
        color = texture(texture0, uv).rgb;
    }

    vec3 blur = soft_blur(uv);
    color = mix(color, blur, 0.08);
    color = tone_curve(color);

    if (scanlineIntensity > 0.0) {
        float lines = scanline(uv);
        color *= 1.0 - scanlineIntensity * (1.0 - lines);
    }

    if (noiseIntensity > 0.0) {
        float n = noise(uv * (time * 0.5 + 10.0)) * noiseIntensity;
        color += vec3(n);
    }

    if (vignetteIntensity > 0.0) {
        color *= vignette(uv);
    }

    finalColor = vec4(color, 1.0);
}
)";
}

void CRTEffect::on_init() {
    m_shader_loaded = false;
    m_time = 0.0F;
    load_shader();
}

void CRTEffect::on_update() {
    if (enabled && m_shader_loaded) {
        m_time += get_frame_time();
        update_shader_uniforms();
    }
}

void CRTEffect::load_shader() {
    m_shader = LoadShaderFromMemory(nullptr, get_fragment_shader_code());

    if (m_shader.id == 0) {
        log_error("Failed to compile CRT shader");
        return;
    }

    m_shader_loaded = true;
    log_info("CRT shader loaded successfully");
}

void CRTEffect::update_shader_uniforms() {
    if (!m_shader_loaded) {
        return;
    }

    const int time_loc = GetShaderLocation(m_shader, "time");
    const int resolution_loc = GetShaderLocation(m_shader, "resolution");
    const int curvature_loc = GetShaderLocation(m_shader, "curvature");
    const int scanline_loc = GetShaderLocation(m_shader, "scanlineIntensity");
    const int vignette_loc = GetShaderLocation(m_shader, "vignetteIntensity");
    const int noise_loc = GetShaderLocation(m_shader, "noiseIntensity");
    const int chromatic_loc = GetShaderLocation(m_shader, "chromaticAberration");

    SetShaderValue(m_shader, time_loc, &m_time, SHADER_UNIFORM_FLOAT);

    const float resolution[2] = {get_screen_width(), get_screen_height()};
    SetShaderValue(m_shader, resolution_loc, resolution, SHADER_UNIFORM_VEC2);

    SetShaderValue(m_shader, curvature_loc, &curvature, SHADER_UNIFORM_FLOAT);
    SetShaderValue(m_shader, scanline_loc, &scanline_intensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(m_shader, vignette_loc, &vignette_intensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(m_shader, noise_loc, &noise_intensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(m_shader, chromatic_loc, &chromatic_aberration, SHADER_UNIFORM_FLOAT);
}
