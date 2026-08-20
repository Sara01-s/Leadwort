#type vertex
#version 450 core

layout(location = 0) in vec3 a_position;

uniform mat4 _ViewMatrix;
uniform mat4 _ProjectionMatrix;

out vec3 v_localPos;

void main() {
	v_localPos = a_position;
	gl_Position = _ProjectionMatrix * _ViewMatrix * vec4(a_position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 fragColor;

uniform samplerCube _EnvironmentMap;
uniform float _Roughness;

in vec3 v_localPos;

const float PI = 3.14159265359;
const uint SAMPLE_COUNT = 1024u;

// Van der Corput + Hammersley
float radicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);

	return float(bits) * 2.3283064365386963e-10;
}

vec2 hammersley(uint i, uint n) {
	return vec2(float(i) / float(n), radicalInverse_VdC(i));
}

// GGX Importance-sampling around N
vec3 importanceSampleGGX(vec2 xi, vec3 n, float roughness) {
	float a = roughness * roughness;

	float phi = 2.0 * PI * xi.x;
	float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	vec3 h = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

	vec3 up = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, n));
	vec3 bitangent = cross(n, tangent);

	return normalize(tangent * h.x + bitangent * h.y + n * h.z);
}

void main() {
	vec3 n = normalize(v_localPos);
	vec3 r = n;
	vec3 v = r; // view = normal (standard approximation for pre filtering)

	float totalWeight = 0.0;
	vec3 prefilteredColor = vec3(0.0);

	for (uint i = 0u; i < SAMPLE_COUNT; i++) {
		vec2 xi = hammersley(i, SAMPLE_COUNT);
		vec3 h = importanceSampleGGX(xi, n, _Roughness);
		vec3 l = normalize(2.0 * dot(v, h) * h - v);

		float nDotL = max(dot(n, l), 0.0);
		if (nDotL > 0.0) {
			prefilteredColor += texture(_EnvironmentMap, l).rgb * nDotL;
			totalWeight += nDotL;
		}
	}

	prefilteredColor = totalWeight > 0.0 ? prefilteredColor / totalWeight : vec3(0.0);
	fragColor = vec4(prefilteredColor, 1.0);
}