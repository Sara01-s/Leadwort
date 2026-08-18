#ifndef LW_LIGHTING_HLSL
#define LW_LIGHTING_HLSL

#include "shd_pbr.glsl"
#include "shd_lighting_data.glsl"

layout(binding = 15) uniform sampler2DShadow _ShadowMap;

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

float calcShadow(vec4 lightSpacePos, vec3 N, vec3 L) {
	vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
	projCoords = projCoords * 0.5 + 0.5; // [-1,1] -> [0,1]

	if (projCoords.z > 1.0) {
		return 0.0;
	}

	float bias = max(0.005 * (1.0 - dot(N, L)), 0.0015);

	// Profundidad de referencia aplicando el sesgo
	float currentDepth = projCoords.z - bias;

	vec2 texelSize = 1.0 / vec2(textureSize(_ShadowMap, 0));
	float shadow = 0.0;

	// PCF 3x3
	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			vec2 offset = vec2(x, y) * texelSize;
			shadow += texture(_ShadowMap, vec3(projCoords.xy + offset, currentDepth));
		}
	}

	shadow /= 9.0;

	return 1.0 - shadow;

	return shadow;
}

#endif