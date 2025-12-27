#include "game/crt_effect.h"
#include "core/raylib_wrapper.h"
#include "remote_logger/remote_logger.h"

const char* CRTEffect::get_fragment_shader_code() {
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
    float vig = uv.x * uv.y * 15.0;
    return pow(vig, vignetteIntensity);
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
        float offset = chromaticAberration * 0.01;
        color.r = texture(texture0, vec2(uv.x + offset, uv.y)).r;
        color.g = texture(texture0, uv).g;
        color.b = texture(texture0, vec2(uv.x - offset, uv.y)).b;
    } else {
        color = texture(texture0, uv).rgb;
    }
    
    if (scanlineIntensity > 0.0) {
        float scanline = sin(uv.y * resolution.y * 2.0) * 0.5 + 0.5;
        color *= 1.0 - scanlineIntensity * (1.0 - scanline);
    }
    
    if (noiseIntensity > 0.0) {
        float n = noise(uv * time) * noiseIntensity;
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
    m_time = 0.0f;
    load_shader();
}

void CRTEffect::on_update() {
    if (enabled && m_shader_loaded) {
        m_time += get_frame_time();
        update_shader_uniforms();
    }
}

void CRTEffect::load_shader() {
    try {
        m_shader = LoadShaderFromMemory(nullptr, get_fragment_shader_code());
        
        if (m_shader.id == 0) {
            log_error() << "Failed to compile CRT shader" << std::endl;
            return;
        }
        
        m_shader_loaded = true;
        log_info() << "CRT shader loaded successfully" << std::endl;
        
    } catch (...) {
        log_error() << "Exception while loading CRT shader" << std::endl;
    }
}

void CRTEffect::update_shader_uniforms() {
    if (!m_shader_loaded) return;
    
    int timeLoc = GetShaderLocation(m_shader, "time");
    int resolutionLoc = GetShaderLocation(m_shader, "resolution");
    int curvatureLoc = GetShaderLocation(m_shader, "curvature");
    int scanlineLoc = GetShaderLocation(m_shader, "scanlineIntensity");
    int vignetteLoc = GetShaderLocation(m_shader, "vignetteIntensity");
    int noiseLoc = GetShaderLocation(m_shader, "noiseIntensity");
    int chromaticLoc = GetShaderLocation(m_shader, "chromaticAberration");
    
    SetShaderValue(m_shader, timeLoc, &m_time, SHADER_UNIFORM_FLOAT);
    
    float resolution[2] = {(float)VIRTUAL_WIDTH, (float)VIRTUAL_HEIGHT};
    SetShaderValue(m_shader, resolutionLoc, resolution, SHADER_UNIFORM_VEC2);
    
    SetShaderValue(m_shader, curvatureLoc, &curvature, SHADER_UNIFORM_FLOAT);
    SetShaderValue(m_shader, scanlineLoc, &scanline_intensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(m_shader, vignetteLoc, &vignette_intensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(m_shader, noiseLoc, &noise_intensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(m_shader, chromaticLoc, &chromatic_aberration, SHADER_UNIFORM_FLOAT);
}
