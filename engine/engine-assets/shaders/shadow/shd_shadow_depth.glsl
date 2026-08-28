#type vertex
#version 430 core

layout (location = 0) in vec3 a_position;
// Location 2 to match the lit shader's layout (position, normal, uv, ...). Meshes with
// no UVs leave it at zero, which is fine: those never have a base colour texture and so
// are never alpha tested.
layout (location = 2) in vec2 a_uv;

uniform mat4 _ModelMatrix;
uniform mat4 _LightSpaceMatrix;

out vec2 v_uv;

void main() {
	v_uv = a_uv;
	gl_Position = _LightSpaceMatrix * _ModelMatrix * vec4(a_position, 1.0);
}

#type fragment
#version 430 core

in vec2 v_uv;

uniform sampler2D _DiffuseTexture;
uniform float _AlphaCutoff; // 0 = no alpha test

void main() {
	// Cutout foliage has to run the same test as the lit pass, or every leaf casts the
	// shadow of the quad it is painted on.
	if (_AlphaCutoff > 0.0 && texture(_DiffuseTexture, v_uv).a < _AlphaCutoff) {
		discard;
	}

	// Depth-only: no color output needed, gl_FragDepth writes automatically.
}