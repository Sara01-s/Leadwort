#type vertex
#version 430 core

layout (location = 0) in vec3 a_position;

uniform mat4 _ModelMatrix;
uniform mat4 _LightSpaceMatrix;

void main() {
	gl_Position = _LightSpaceMatrix * _ModelMatrix * vec4(a_position, 1.0);
}

#type fragment
#version 430 core

void main() {
	// Depth-only: no color output needed, gl_FragDepth writes automatically.
}