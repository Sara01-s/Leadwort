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

uniform sampler2D _EquirectangularMap;

in vec3 v_localPos;

const vec2 INV_ATAN = vec2(0.1591, 0.3183);

vec2 sampleSphericalMap(vec3 dir) {
	vec2 uv = vec2(atan(dir.z, dir.x), asin(dir.y));

	uv *= INV_ATAN;
	uv += 0.5;

	return uv;
}

void main() {
	vec2 uv = sampleSphericalMap(normalize(v_localPos));
	vec3 color = texture(_EquirectangularMap, uv).rgb;

	fragColor = vec4(color, 1.0);
}