#ifndef LW_LIGHTING_HLSL
#define LW_LIGHTING_HLSL

#include "shd_pbr.glsl"

#define MAX_POINT_LIGHTS 8
#define MAX_SPOT_LIGHTS 8

struct PointLight {
	vec4 position;
	vec4 color;
	vec4 attenuation;
};

struct SpotLight {
	vec4 position;
	vec4 direction;
	vec4 color;
	vec4 attenuation;
	vec4 cutoffs;
};

layout(std140, binding = 1) uniform LightingData {
	vec4 _LightDirection;      // w ignored (directional)
	vec4 _LightColorIntensity; // rgb = color, a = intensity (directional)

	PointLight _PointLights[MAX_POINT_LIGHTS];
	SpotLight  _SpotLights[MAX_SPOT_LIGHTS];

	ivec4 _LightCounts; // x = numPointLights, y = numSpotLights, zw unused
};

float calcAttenuation(vec3 att, float distance) {
	return 1.0 / (att.x + att.y * distance + att.z * distance * distance);
}

vec3 calcPointLight(PointLight light, vec3 F0, vec3 albedo, vec3 N, vec3 V, float roughness, float metallic, vec3 worldPos) {
	vec3 L = light.position.xyz - worldPos;
	float distance = length(L);
	L = normalize(L);

	vec3 sum = V + L;
	vec3 H = dot(sum, sum) > 1e-6 ? normalize(sum) : N;

	float attenuation = calcAttenuation(light.attenuation.xyz, distance);
	vec3 lightColor = light.color.rgb * light.color.a * attenuation;

	return PBR_Direct(F0, albedo, N, V, L, H, roughness, metallic, lightColor);
}

vec3 calcSpotLight(SpotLight light, vec3 F0, vec3 albedo, vec3 N, vec3 V, float roughness, float metallic, vec3 worldPos) {
	vec3 L = light.position.xyz - worldPos;
	float distance = length(L);
	L = normalize(L);

	vec3 sum = V + L;
	vec3 H = dot(sum, sum) > 1e-6 ? normalize(sum) : N;

	float attenuation = calcAttenuation(light.attenuation.xyz, distance);

	float theta = dot(L, normalize(-light.direction.xyz));
	float epsilon = light.cutoffs.x - light.cutoffs.y; // inner - outer
	float spotIntensity = clamp((theta - light.cutoffs.y) / max(epsilon, 1e-4), 0.0, 1.0);

	vec3 lightColor = light.color.rgb * light.color.a * attenuation * spotIntensity;

	return PBR_Direct(F0, albedo, N, V, L, H, roughness, metallic, lightColor);
}

#endif