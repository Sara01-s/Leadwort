#type vertex
#version 430 core

layout (location = 0) in vec3 a_position;
layout (location = 2) in vec2 a_uv;

out vec2 v_uv;

void main() {
    v_uv = a_uv;
    gl_Position = vec4(a_position.xy, 0.0, 1.0);
}

#type fragment
#version 430 core

#include "include/shd_post_process_effects.glsl"

out vec4 fragColor;
in vec2 v_uv;

uniform sampler2D _ScreenTexture;
uniform sampler2D _DepthTexture;
uniform vec4 _ScreenParams;  // x = width pixels, y = height pixels, z = near plane, w = far plane

layout (std140, binding = 0) uniform CameraData {
    mat4 _ViewMatrix;
    mat4 _ProjectionMatrix;
    vec4 _CameraPosition;
    vec4 _Padding;
};

uniform mat4 _InvViewProjectionMatrix;

// Depth of Field
const float _FocusDistance = 5.0;
const float _FocusRange = 3.0;
const float _MaxBlurRadius = 8.0;

// Bloom
const float _BloomThreshold = 1.0;
const float _BloomIntensity = 0.6;
const float _BloomRadius = 12.0;

// Screen-space fog
const vec3  _FogColor        = vec3(0.55, 0.65, 0.75);
const float _FogHeight       = 500.0;
const float _FogAttenuation  = 1.2;
const float _FogDensity      = 0.006;
const float _FogOffset       = 30.0;

const float _VignetteIntensity = 1.1;
const float _Gamma = 1/2.2;
const float _Exposure = 0.1;
const float _Brightness = 0.3;
const float _GrainStrength = 0.01;
const float _ChromaticAberrationOffset = 0.5;
const float _ChromaticAberrationIntensity = 0.0; // [0, 1]

void main() {
    vec2 uv = v_uv;
    vec2 texelSize = 1.0 / _ScreenParams.xy;

    vec3 sceneColor = texture(_ScreenTexture, uv).rgb;
    float depth = texture(_DepthTexture, uv).r;

    sceneColor = mix(sceneColor, applyChromaticAberration(_ScreenTexture, uv, _ChromaticAberrationOffset), _ChromaticAberrationIntensity);
    sceneColor = applyExposure(sceneColor, _Exposure);

    //sceneColor = applyDepthOfField(
    //    _ScreenTexture, _DepthTexture, uv, texelSize,
    //    _FocusDistance, _FocusRange, _MaxBlurRadius,
    //    _ScreenParams.z, _ScreenParams.w
    //);

    sceneColor += applyBloom(_ScreenTexture, uv, texelSize, _BloomThreshold, _BloomIntensity, _BloomRadius);

    if (depth < 1.0) { // no geometry
        vec3 worldPos = reconstructWorldPos(uv, depth, _InvViewProjectionMatrix);
        sceneColor = applyScreenSpaceFog(
                sceneColor, worldPos, _CameraPosition.xyz,
                _FogColor, _FogHeight, _FogAttenuation, _FogDensity, _FogOffset
        );
    }

    sceneColor = applyACES(sceneColor);
    sceneColor = applyBrightness(sceneColor, _Brightness);

    sceneColor *= applyVignette(uv, _VignetteIntensity);
    sceneColor = applyFilmGrain(sceneColor, uv, _GrainStrength);

    sceneColor = applyGammaCorrection(sceneColor, _Gamma);

    fragColor = vec4(sceneColor, 1.0);
}