#type vertex
#version 450 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

uniform mat4 _ModelMatrix;
uniform mat3 _NormalMatrix;

layout (std140, binding = 0) uniform CameraData {
	mat4 _ViewMatrix;
	mat4 _ProjectionMatrix;
	vec4 _CameraPosition;
	vec4 _Padding;
};

layout(std140, binding = 2) uniform TimeData {
	vec4 _Time; // x = t, y = t/2, z = t*2, w = deltaTime
};

// Wave FBM parameters
const int   WAVE_COUNT           = 8;      // vertexWaveCount
const float WAVE_SEED            = 0.0;
const float WAVE_SEED_ITER       = 1253.2131;
const float WAVE_FREQUENCY       = 0.02;
const float WAVE_FREQUENCY_MULT  = 1.18;
const float WAVE_AMPLITUDE       = 1.0;
const float WAVE_AMPLITUDE_MULT  = 0.82;
const float WAVE_SPEED           = 0.05;    // vertexInitialSpeed
const float WAVE_SPEED_RAMP      = 1.02;
const float WAVE_DRAG            = 1.0;
const float WAVE_HEIGHT          = 2.5;
const float WAVE_MAX_PEAK        = 0.9;
const float WAVE_PEAK_OFFSET     = 1.0;

out vec2 v_uv;
out vec3 v_worldPos;
out vec3 v_normal;
out float v_height;

// Returns (height, dHeight/dx, dHeight/dz) via analytic partial derivatives —
// no central-difference sampling needed.
vec3 WaveFBM(vec2 p) {
	float f = WAVE_FREQUENCY;
	float a = WAVE_AMPLITUDE;
	float speed = WAVE_SPEED;
	float seed = WAVE_SEED;

	float h = 0.0;
	vec2 grad = vec2(0.0);
	float amplitudeSum = 0.0;

	for (int i = 0; i < WAVE_COUNT; i++) {
		vec2 d = normalize(vec2(cos(seed), sin(seed)));

		float x = dot(d, p) * f + _Time.x * speed;
		float wave = a * exp(WAVE_MAX_PEAK * sin(x) - WAVE_PEAK_OFFSET);

		// dWave/dx via chain rule: d/dx[a * exp(k*sin(x) - c)] = wave * k * cos(x)
		float dw = WAVE_MAX_PEAK * wave * cos(x) * f;

		h += wave;
		grad += d * dw;

		// Domain warp: drag the sample point along the wave gradient,
		// sharpens wave crests.
		p += d * -dw * a * WAVE_DRAG;

		amplitudeSum += a;
		f     *= WAVE_FREQUENCY_MULT;
		a     *= WAVE_AMPLITUDE_MULT;
		speed *= WAVE_SPEED_RAMP;
		seed  += WAVE_SEED_ITER;
	}

	vec3 result = vec3(h, grad.x, grad.y) / max(amplitudeSum, 0.0001);
	result *= WAVE_HEIGHT;

	return result;
}

void main() {
	v_uv = a_uv;

	vec4 worldPos4 = _ModelMatrix * vec4(a_position, 1.0);

	vec3 fbm = WaveFBM(worldPos4.xz);
	worldPos4.y += fbm.x;

	// Surface normal from analytic partial derivatives:
	// height field h(x,z) -> normal = normalize(-dh/dx, 1, -dh/dz)
	v_normal = normalize(_NormalMatrix * normalize(vec3(-fbm.y, 1.0, -fbm.z)));

	v_height = fbm.x;
	v_worldPos = worldPos4.xyz;
	gl_Position = _ProjectionMatrix * _ViewMatrix * worldPos4;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 fragColor;

uniform vec4 _Color;

layout (std140, binding = 0) uniform CameraData {
	mat4 _ViewMatrix;
	mat4 _ProjectionMatrix;
	vec4 _CameraPosition;
	vec4 _Padding;
};

layout(std140, binding = 2) uniform TimeData {
	vec4 _Time;
};

layout(binding = 13) uniform samplerCube _IBLPrefilter;

const float PI = 3.14159265358979323846;

const vec3  SUN_DIRECTION        = vec3(0.3, -0.15, 0.9);
const vec3  SUN_COLOR            = vec3(1.0, 0.95, 0.85);

// Blinn-Phong reflectance terms
const vec3  AMBIENT_COLOR        = vec3(0.02, 0.2, 0.3);
const vec3  DIFFUSE_REFLECTANCE  = vec3(0.12, 0.1, 0.2);
const vec3  SPECULAR_REFLECTANCE = vec3(1.0, 1.0, 0.95);
const float SHININESS            = 200.0;

// Per-term normal shaping: flattens/sharpens the normal for each lighting term
const float NORMAL_STRENGTH          = 1.0;
const float FRESNEL_NORMAL_STRENGTH  = 1.0;
const float SPECULAR_NORMAL_STRENGTH = 1.0;

// Schlick fresnel: R scales every reflected term
const vec3  FRESNEL_COLOR        = vec3(0.35, 0.50, 0.60);
const float FRESNEL_BIAS         = 0.05;
const float FRESNEL_STRENGTH     = 1.0;
const float FRESNEL_SHININESS    = 5.0;
const bool  USE_ENVIRONMENT_MAP  = true;

const float SUN_DISC_EXPONENT    = 500.0;
const float SUN_DISC_INTENSITY   = 1.0;

const vec3  TIP_COLOR            = vec3(0.9, 0.95, 1.0);
const float TIP_ATTENUATION      = 4.0;

const vec3  FOG_COLOR            = vec3(0.5, 0.7, 0.9);
const float FOG_DENSITY          = 0.008;
const float FOG_OFFSET           = 20.0;

const int   FRAG_WAVE_COUNT      = 40;
const float FRAG_SEED            = 0.0;
const float FRAG_SEED_ITER       = 1253.2131;
const float FRAG_FREQUENCY       = 0.6;
const float FRAG_FREQUENCY_MULT  = 1.18;
const float FRAG_AMPLITUDE       = 1.0;
const float FRAG_AMPLITUDE_MULT  = 0.82;
const float FRAG_SPEED           = 2.0;
const float FRAG_SPEED_RAMP      = 1.07;
const float FRAG_DRAG            = 1.0;
const float FRAG_HEIGHT          = 1.0;
const float FRAG_MAX_PEAK        = 1.0;
const float FRAG_PEAK_OFFSET     = 0.9;

in vec2 v_uv;
in vec3 v_worldPos;
in vec3 v_normal;
in float v_height;

vec3 FragmentWaveFBM(vec2 p) {
	float f = FRAG_FREQUENCY;
	float a = FRAG_AMPLITUDE;
	float speed = FRAG_SPEED;
	float seed = FRAG_SEED;

	float h = 0.0;
	vec2 grad = vec2(0.0);
	float amplitudeSum = 0.0;

	for (int i = 0; i < FRAG_WAVE_COUNT; i++) {
		vec2 d = normalize(vec2(cos(seed), sin(seed)));

		float x = dot(d, p) * f + _Time.x * speed;
		float wave = a * exp(FRAG_MAX_PEAK * sin(x) - FRAG_PEAK_OFFSET);
		float dw = FRAG_MAX_PEAK * wave * cos(x) * f;

		h += wave;
		grad += d * dw;

		p += d * -dw * a * FRAG_DRAG;

		amplitudeSum += a;
		f     *= FRAG_FREQUENCY_MULT;
		a     *= FRAG_AMPLITUDE_MULT;
		speed *= FRAG_SPEED_RAMP;
		seed  += FRAG_SEED_ITER;
	}

	vec3 result = vec3(h, grad.x, grad.y) / max(amplitudeSum, 0.0001);
	result.x *= FRAG_HEIGHT;

	return result;
}

void main() {
	vec3 fineFbm = FragmentWaveFBM(v_worldPos.xz);
	vec3 fineNormal = normalize(vec3(-fineFbm.y, 1.0, -fineFbm.z));

	vec3 normal = normalize(v_normal + fineNormal - vec3(0.0, 1.0, 0.0));
	normal.xz *= NORMAL_STRENGTH;
	normal = normalize(normal);

	vec3 lightDir = normalize(-SUN_DIRECTION);
	vec3 viewDir = normalize(_CameraPosition.xyz - v_worldPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	float ndotl = max(dot(lightDir, normal), 0.0);

	// Lambert diffuse
	vec3 diffuse = SUN_COLOR * ndotl * (DIFFUSE_REFLECTANCE / PI);

	// Schlick fresnel on its own shaped normal
	vec3 fresnelNormal = normal;
	fresnelNormal.xz *= FRESNEL_NORMAL_STRENGTH;
	fresnelNormal = normalize(fresnelNormal);

	float base = 1.0 - max(dot(viewDir, fresnelNormal), 0.0);
	float exponential = pow(base, FRESNEL_SHININESS);
	float R = exponential + FRESNEL_BIAS * (1.0 - exponential);
	R *= FRESNEL_STRENGTH;

	vec3 fresnel = FRESNEL_COLOR * R;

	if (USE_ENVIRONMENT_MAP) {
		// Sky reflection and sun disc are both reflected light, so both scale by R
		vec3 reflectedDir = reflect(-viewDir, normal);
		vec3 skyColor = textureLod(_IBLPrefilter, reflectedDir, 0.0).rgb;
		float sunDisc = pow(max(dot(reflectedDir, lightDir), 0.0), SUN_DISC_EXPONENT);
		vec3 sun = SUN_COLOR * sunDisc * SUN_DISC_INTENSITY;

		fresnel = skyColor * R;
		fresnel += sun * R;
	}

	// Blinn-Phong specular, gated by ndotl and by its own Schlick term
	vec3 specNormal = normal;
	specNormal.xz *= SPECULAR_NORMAL_STRENGTH;
	specNormal = normalize(specNormal);

	float spec = pow(max(dot(specNormal, halfwayDir), 0.0), SHININESS) * ndotl;
	vec3 specular = SUN_COLOR * SPECULAR_REFLECTANCE * spec;

	base = 1.0 - max(dot(viewDir, halfwayDir), 0.0);
	exponential = pow(base, 5.0);
	R = exponential + FRESNEL_BIAS * (1.0 - exponential);

	specular *= R;

	vec3 tip = TIP_COLOR * pow(max(fineFbm.x, 0.0), TIP_ATTENUATION);

	vec3 colorBeforeFog = AMBIENT_COLOR + diffuse + specular + fresnel + tip;

	float viewDistance = length(_CameraPosition.xyz - v_worldPos);
	float fogFactor = FOG_DENSITY * max(0.0, viewDistance - FOG_OFFSET);
	fogFactor = exp2(-fogFactor * fogFactor);

	vec3 finalColor = mix(FOG_COLOR, colorBeforeFog, clamp(fogFactor, 0.0, 1.0));
	fragColor = vec4(pow(finalColor, vec3(1.0/2.2)), 1.0) * _Color;

	fragColor = vec4(finalColor, 1.0) * _Color;
}
