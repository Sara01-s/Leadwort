#type vertex
#version 450 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

#ifdef HAS_TANGENTS
    layout(location = 3) in vec3 a_tangent;
    layout(location = 4) in vec3 a_bitangent;
#endif

uniform mat4 _ModelMatrix;
uniform mat3 _NormalMatrix;

layout (std140, binding = 0) uniform CameraData {
    mat4 _ViewMatrix;
    mat4 _ProjectionMatrix;
    vec4 _CameraPosition;
    vec4 _Padding;
};

out vec3 v_worldPosition;
out vec3 v_worldNormal;
out vec2 v_uv;
out vec3 v_cameraPosition;

#ifdef HAS_TANGENTS
    out mat3 v_tbn;
#endif

void main() {
    vec4 worldPos = _ModelMatrix * vec4(a_position, 1.0);
    v_worldPosition = worldPos.xyz;
    v_uv = a_uv;

    mat3 normalMatrix = transpose(inverse(mat3(_ModelMatrix)));
    vec3 N = normalize(normalMatrix * a_normal);
    v_worldNormal = N;

    #ifdef HAS_TANGENTS
        vec3 T = normalize(_NormalMatrix * a_tangent);
        vec3 B = normalize(_NormalMatrix * a_bitangent);
        v_tbn = mat3(T, B, v_worldNormal);
    #endif

    v_cameraPosition = _CameraPosition.xyz;

    gl_Position = _ProjectionMatrix * _ViewMatrix * worldPos;
}

#type fragment
#version 450 core
#include "include/shd_pbr.glsl"
#include "include/shd_lighting.glsl"

layout(location = 0) out vec4 fragColor;

void main() {
    vec2 uv = v_uv;

    vec3  albedo    = sampleAlbedo(uv);
    float alpha     = sampleAlpha(uv);
    float roughness = sampleRoughness(uv);
    float metallic  = sampleMetallic(uv);

    if (alpha < 0.1) {
        discard;
    }

    vec3  N        = sampleNormal(uv);
    vec3  emission = sampleEmission(uv);
    vec3  F0       = sampleBaseReflectivity(uv, albedo, metallic);
    float ao       = sampleAO(uv);

    vec3 V = normalize(v_cameraPosition - v_worldPosition);

    vec3 directLighting = vec3(0.0);

    // Directional light
    {
        vec3 L = normalize(-_LightDirection.xyz);
        vec3 sum = V + L;
        vec3 H = dot(sum, sum) > 1e-6 ? normalize(sum) : N;

        vec3 lightColor = _LightColorIntensity.rgb * _LightColorIntensity.a;
        directLighting += PBR_Direct(F0, albedo, N, V, L, H, roughness, metallic, lightColor);
    }

    // Point lights
    for (int i = 0; i < _LightCounts.x; i++) {
        directLighting += calcPointLight(_PointLights[i], F0, albedo, N, V, roughness, metallic, v_worldPosition);
    }

    // Spot lights
    for (int i = 0; i < _LightCounts.y; i++) {
        directLighting += calcSpotLight(_SpotLights[i], F0, albedo, N, V, roughness, metallic, v_worldPosition);
    }

    vec3 ambientLight = AMBIENT_LIGHT * albedo * ao;
    vec3 color = emission + ambientLight + directLighting;

    color = pow(color, vec3(1.0 / 2.2));

    fragColor = vec4(color, alpha);
}