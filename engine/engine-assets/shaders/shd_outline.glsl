#type vertex
#version 450 core

layout(location = 0) in vec3 a_position;

uniform mat4 _ModelMatrix;
uniform float _OutlineThickness;

layout (std140, binding = 0) uniform CameraData {
	mat4 _ViewMatrix;
	mat4 _ProjectionMatrix;
	vec4 _CameraPosition;
	vec4 _Padding;
};

void main() {
	vec4 worldPos = _ModelMatrix * vec4(a_position, 1.0);
	vec4 viewPos  = _ViewMatrix * worldPos;

	vec4 objectCenterView = _ViewMatrix * (_ModelMatrix * vec4(0.0, 0.0, 0.0, 1.0));
	vec3 dirFromCenter = normalize(viewPos.xyz - objectCenterView.xyz);

	float scale = -viewPos.z * _OutlineThickness * 0.05;
	viewPos.xyz += dirFromCenter * scale;

	gl_Position = _ProjectionMatrix * viewPos;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 fragColor;

uniform vec4 _OutlineColor;

void main() {
	fragColor = _OutlineColor;
}