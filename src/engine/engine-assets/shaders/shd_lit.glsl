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

layout(location = 0) out vec4 fragColor;

in vec3 v_worldPosition;
in vec3 v_worldNormal;
in vec2 v_uv;
in vec3 v_cameraPosition;

#ifdef HAS_TANGENTS
in mat3 v_tbn;
#endif

uniform vec3  _LightDirection;
uniform float _LightIntensity;
uniform vec3  _LightColor;
uniform vec4  _Color;
uniform float _MetallicIntensity;
uniform float _RoughnessIntensity;

#ifdef HAS_DIFFUSE
uniform sampler2D _DiffuseTexture;
#endif
#ifdef HAS_SPECULAR
uniform sampler2D _SpecularTexture;  // M/R: specular color | S/G: specular color (RGB) + glossiness (A)
#endif
#if defined(HAS_NORMAL) && defined(HAS_TANGENTS)
uniform sampler2D _NormalTexture;
#endif
#ifdef HAS_OPACITY
uniform sampler2D _OpacityTexture;
#endif
#ifdef HAS_EMISSIVE
uniform sampler2D _EmissiveTexture;
#endif
#ifdef HAS_ROUGHNESS
uniform sampler2D _RoughnessTexture;
#endif
#ifdef HAS_METALLIC
uniform sampler2D _MetallicTexture;
#endif
#ifdef HAS_AO
uniform sampler2D _AmbientOcclusionTexture;
#endif

const vec3 AMBIENT_LIGHT = vec3(0.03, 0.03, 0.04);

vec3 linearColorSpace(vec3 color) {
    return pow(color, vec3(2.2));
}

// ----------------------------------------------------------------

vec3 sampleAlbedo(vec2 uv) {
    vec3 albedo = linearColorSpace(_Color.rgb);
    #ifdef HAS_DIFFUSE
    albedo *= linearColorSpace(texture(_DiffuseTexture, uv).rgb);
    #endif
    return albedo;
}

float sampleAlpha(vec2 uv) {
    float alpha = _Color.a;
    #if defined(HAS_DIFFUSE) && !defined(HAS_OPACITY)
    alpha *= texture(_DiffuseTexture, uv).a;
    #endif
    #ifdef HAS_OPACITY
    alpha *= texture(_OpacityTexture, uv).a;
    #endif
    return alpha;
}

float sampleRoughness(vec2 uv) {
    float roughness = 0.5;

    #ifdef SPECULAR_GLOSSINESS
    #ifdef HAS_SPECULAR
    roughness = max(1.0 - texture(_SpecularTexture, uv).a, 0.05);
    #endif
    #else
    roughness = max(_RoughnessIntensity, 0.05);
    #ifdef HAS_ROUGHNESS
    roughness *= texture(_RoughnessTexture, uv).g;
    #endif
    #endif

    return roughness;
}

float sampleMetallic(vec2 uv) {
    float metallic = 0.0;

    #ifndef SPECULAR_GLOSSINESS
    metallic = _MetallicIntensity;
    #ifdef HAS_METALLIC
    metallic *= texture(_MetallicTexture, uv).b;
    #endif
    #endif

    return metallic;
}

vec3 sampleNormal(vec2 uv) {
    vec3 N = normalize(v_worldNormal);
    #if defined(HAS_NORMAL) && defined(HAS_TANGENTS)
    vec3 tangentNormal = texture(_NormalTexture, uv).xyz * 2.0 - 1.0;
    N = normalize(v_tbn * tangentNormal);
    #endif
    return N;
}

vec3 sampleEmission(vec2 uv) {
    vec3 emission = vec3(0.0);

        #ifdef HAS_EMISSIVE
            emission = texture(_EmissiveTexture, uv).rgb;
        #endif

    return emission;
}

// F0 = base reflectivity, the reflectivity when the viewing angle is perpendicular to the surface.
vec3 sampleBaseReflectivity(vec2 uv, vec3 albedo, float metallic) {
    vec3 F0 = vec3(0.04);

    #ifdef SPECULAR_GLOSSINESS
    #ifdef HAS_SPECULAR
    F0 = linearColorSpace(texture(_SpecularTexture, uv).rgb);
    #endif
    #else
    F0 = mix(vec3(0.04), albedo, metallic);
    #ifdef HAS_SPECULAR
    vec3 specularSample = linearColorSpace(texture(_SpecularTexture, uv).rgb);
    F0 = mix(specularSample, albedo * specularSample, metallic);
    #endif
    #endif

    return F0;
}

float sampleAO(vec2 uv) {
    float ao = 1.0;
    #ifdef HAS_AO
    ao = texture(_AmbientOcclusionTexture, uv).r;
    #endif
    return ao;
}

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
    vec3 L = normalize(-_LightDirection);
    vec3 sum = V + L;
    vec3 H = dot(sum, sum) > 1e-6 ? normalize(sum) : N;

    vec3 lightColor = _LightColor * _LightIntensity;
    vec3 color = PBR(F0, albedo, N, V, L, H, roughness, metallic, lightColor, emission, ao, AMBIENT_LIGHT);

    color = pow(color, vec3(1.0 / 2.2));

    fragColor = vec4(color, alpha);
}