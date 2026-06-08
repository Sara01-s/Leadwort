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

layout (std140, binding = 0) uniform CameraData {
    mat4 _ViewMatrix;
    mat4 _ProjectionMatrix;
    vec4 _CameraPosition;
    vec4 _Padding;
};

out vec3 v_worldPosition;
out vec3 v_viewPosition;
out vec3 v_worldNormal;
out vec3 v_viewNormal;
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
        vec3 T = normalize(normalMatrix * a_tangent);
        vec3 B = normalize(normalMatrix * a_bitangent);
        v_tbn = mat3(T, B, N);
    #endif

    v_viewPosition = (_ViewMatrix * worldPos).xyz;
    v_cameraPosition = _CameraPosition.xyz;

    mat3 viewNormalMatrix = transpose(inverse(mat3(_ViewMatrix * _ModelMatrix)));
    v_viewNormal = normalize(viewNormalMatrix * a_normal);

    gl_Position = _ProjectionMatrix * _ViewMatrix * worldPos;
}

#type fragment
#version 450 core
#include "include/shd_pbr.glsl"

layout(location = 0) out vec4 fragColor;

in vec3 v_worldPosition;
in vec3 v_viewPosition;
in vec3 v_worldNormal;
in vec3 v_viewNormal;
in vec2 v_uv;
in vec3 v_cameraPosition;

#ifdef HAS_TANGENTS
in mat3 v_tbn;
#endif

uniform vec3 _LightDirection;
uniform float _LightIntensity;
uniform vec3 _LightColor;
uniform vec4 _Color;
uniform float _MetallicIntensity;
uniform float _RoughnessIntensity;

#ifdef HAS_DIFFUSE
    uniform sampler2D _DiffuseTexture;
#endif
#ifdef HAS_SPECULAR
    uniform sampler2D _SpecularTexture;
#endif
#if defined(HAS_NORMAL) || defined(HAS_TANGENTS)
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

const vec3 AMBIENT_LIGHT = vec3(0.2, 0.2, 0.25);

void main() {
    vec3 albedo = _Color.rgb;
    float alpha = _Color.a;

    float roughness = _RoughnessIntensity;
    float metallic = _MetallicIntensity;

    #ifdef HAS_DIFFUSE
        vec4 diffuseSample = texture(_DiffuseTexture, v_uv);
        albedo *= diffuseSample.rgb;

        #if !(defined(HAS_OPACITY))
            alpha *= diffuseSample.a;
        #endif
    #endif

    #ifdef HAS_ROUGHNESS
        roughness *= texture(_RoughnessTexture, v_uv).r;
    #endif

    #ifdef HAS_METALLIC
        metallic *= texture(_MetallicTexture, v_uv).r;
    #endif

    #ifdef HAS_OPACITY
        alpha *= texture(_OpacityTexture, v_uv).a;
    #endif

    if (alpha < 0.1) {
        discard;
    }

    vec3 N = normalize(v_worldNormal);

    #ifdef HAS_TANGENTS
        vec3 tangentNormal = texture(_NormalTexture, v_uv).xyz * 2.0 - 1.0;
        N = normalize(v_tbn * tangentNormal);
    #endif

    vec3 emission = vec3(0.0);

    #ifdef HAS_EMISSIVE
        emission = texture(_EmissiveTexture, v_uv).rgb;
    #endif

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    #ifdef HAS_SPECULAR
        vec3 specularSample = texture(_SpecularTexture, v_uv).rgb;
        F0 = specularSample;
    #endif

    float ao = 1.0;

    #ifdef HAS_AO
        ao = texture(_AmbientOcclusionTexture, v_uv).r;
    #endif

    vec3 V = normalize(v_cameraPosition - v_worldPosition);
    vec3 L = normalize(-_LightDirection);
    vec3 H = normalize(V + L);

    vec3 lightColor = _LightColor * _LightIntensity;
    vec3 color = PBR(F0, albedo, N, V, L, H, roughness, metallic, lightColor, emission, ao, AMBIENT_LIGHT);

    fragColor = vec4(color, alpha);
}