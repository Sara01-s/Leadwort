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
		float dw = WAVE_MAX_PEAK * wave * cos(x);

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
	result.x *= WAVE_HEIGHT;

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

const vec3  SUN_DIRECTION        = vec3(0.3, -0.15, 0.9);
const vec3  SUN_COLOR            = vec3(1.0, 0.95, 0.85);

const vec3  DEEP_COLOR    		 = vec3(0.02, 0.03, 0.04);
const vec3  SHALLOW_COLOR  		 = vec3(0.15, 0.17, 0.19);

const vec3  SPECULAR_COLOR       = vec3(1.0, 1.0, 0.95);

const float FRESNEL_BIAS         = 0.05;
const float FRESNEL_STRENGTH     = 1.0;
const float FRESNEL_POWER        = 3.0;

const vec3  TIP_COLOR            = vec3(0.9, 0.95, 1.0);
const float FOAM_THRESHOLD       = 1.5;
const float FOAM_SOFTNESS        = 0.15;

const vec3  FOG_COLOR            = vec3(0.55, 0.65, 0.75);
const float FOG_DENSITY          = 0.008;
const float FOG_OFFSET           = 20.0;

const float SUN_DISC_EXPONENT    = 3500.0;
const float SUN_DISC_INTENSITY   = 8.0;

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
const float FRAG_MAX_PEAK        = 1.0;
const float FRAG_PEAK_OFFSET     = 0.5;

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
		float dw = FRAG_MAX_PEAK * wave * cos(x);

		h += wave;
		grad += d * dw;

		p += d * -dw * a * FRAG_DRAG;

		amplitudeSum += a;
		f     *= FRAG_FREQUENCY_MULT;
		a     *= FRAG_AMPLITUDE_MULT;
		speed *= FRAG_SPEED_RAMP;
		seed  += FRAG_SEED_ITER;
	}

	return vec3(h, grad.x, grad.y) / max(amplitudeSum, 0.0001);
}

void main() {
	vec3 fineFbm = FragmentWaveFBM(v_worldPos.xz);
	vec3 fineNormal = normalize(vec3(-fineFbm.y, 1.0, -fineFbm.z));

	vec3 normal = normalize(v_normal + fineNormal - vec3(0.0, 1.0, 0.0));

	vec3 lightDir = normalize(-SUN_DIRECTION);
	vec3 viewDir = normalize(_CameraPosition.xyz - v_worldPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	float ndotl = max(dot(normal, lightDir), 0.0);

	vec3 baseColor = mix(DEEP_COLOR, SHALLOW_COLOR, ndotl * 0.5 + 0.5);

	float specWide  = pow(max(dot(normal, halfwayDir), 0.0), 60.0);
	float specTight = pow(max(dot(normal, halfwayDir), 0.0), 800.0);
	vec3 specular = SUN_COLOR * SPECULAR_COLOR * (specWide * 0.6 + specTight * 3.0);

	vec3 reflectedDir = reflect(-viewDir, normal);
	float sunDisc = pow(max(dot(reflectedDir, lightDir), 0.0), SUN_DISC_EXPONENT);
	vec3 sun = SUN_COLOR * sunDisc * SUN_DISC_INTENSITY;

	float fresnelBase = 1.0 - max(dot(viewDir, normal), 0.0);
	float fresnel = FRESNEL_BIAS + (1.0 - FRESNEL_BIAS) * pow(fresnelBase, FRESNEL_POWER);
	fresnel *= FRESNEL_STRENGTH;

	vec3 skyColor = textureLod(_IBLPrefilter, reflectedDir, 0.0).rgb;
	vec3 reflection = mix(baseColor, skyColor, clamp(fresnel, 0.0, 1.0));

	float foamAmount = smoothstep(FOAM_THRESHOLD, FOAM_THRESHOLD + FOAM_SOFTNESS, v_height);
	vec3 foam = TIP_COLOR * foamAmount;

	vec3 colorBeforeFog = reflection + specular + sun + foam;

	float viewDistance = length(_CameraPosition.xyz - v_worldPos);
	float fogFactor = FOG_DENSITY * max(0.0, viewDistance - FOG_OFFSET);
	fogFactor = exp2(-fogFactor * fogFactor);

	vec3 finalColor = mix(FOG_COLOR, colorBeforeFog, clamp(fogFactor, 0.0, 1.0));

	fragColor = vec4(finalColor, 1.0) * _Color;
}