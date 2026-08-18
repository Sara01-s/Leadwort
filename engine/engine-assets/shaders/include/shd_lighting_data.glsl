#ifndef LW_LIGHTING_DATA_HLSL
#define LW_LIGHTING_DATA_HLSL

#define MAX_POINT_LIGHTS 8
#define MAX_SPOT_LIGHTS 8

struct PointLight {
	vec4 position;
	vec4 color;       // rgb = color, a = intensity
	vec4 attenuation; // xyz = attenuation
};

struct SpotLight {
	vec4 position;
	vec4 direction;
	vec4 color;       // rgb = color, a = intensity
	vec4 attenuation;
	vec4 cutoffs;     // x = cos(inner), y = cos(outer)
};

layout(std140, binding = 1) uniform LightingData {
	vec4 _LightDirection;      // w ignored (directional)
	vec4 _LightColorIntensity; // rgb = color, a = intensity (directional)

	PointLight _PointLights[MAX_POINT_LIGHTS];
	SpotLight  _SpotLights[MAX_SPOT_LIGHTS];

	mat4 _LightSpaceMatrix;

	ivec4 _LightCounts; // x = numPointLights, y = numSpotLights, zw unused
};

#endif