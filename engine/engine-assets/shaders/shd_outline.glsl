#type vertex
#version 450 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

uniform mat4 _ModelMatrix;
uniform float _OutlineThickness;

layout (std140, binding = 0) uniform CameraData {
	mat4 _ViewMatrix;
	mat4 _ProjectionMatrix;
	vec4 _CameraPosition;
	vec4 _Padding;
};

void main() {
	vec3 safe_normal = length(a_normal) > 0.001 ? normalize(a_normal) : vec3(0.0);

	vec3 extruded_position = a_position + safe_normal * _OutlineThickness;

	vec3 world_position = vec3(_ModelMatrix * vec4(extruded_position, 1.0));
	gl_Position = _ProjectionMatrix * _ViewMatrix * vec4(world_position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 fragColor;

uniform vec4 _OutlineColor;

void main() {
	fragColor = _OutlineColor;
}