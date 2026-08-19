#type vertex
#version 450 core

layout(location = 0) in vec3 a_position;
layout(location = 2) in vec2 a_uv;

uniform mat4 _ModelMatrix;

layout (std140, binding = 0) uniform CameraData {
    mat4 _ViewMatrix;
    mat4 _ProjectionMatrix;
    vec4 _CameraPosition;
    vec4 _Padding;
};

out vec2 v_uv;

void main() {
    v_uv = a_uv;
    mat4 mvp = _ProjectionMatrix * _ViewMatrix * _ModelMatrix;
    gl_Position = mvp * vec4(a_position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 fragColor;

uniform sampler2D _MainTex;
uniform vec4 _Color;

in vec2 v_uv;

void main() {
    fragColor = texture(_MainTex, v_uv) * _Color;
}