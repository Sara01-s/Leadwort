#type vertex
#version 450 core

layout(location = 0) in vec3 a_position;

out vec3 v_texCoords;

layout (std140, binding = 0) uniform CameraData {
    mat4 _ViewMatrix;
    mat4 _ProjectionMatrix;
    vec4 _CameraPosition;
    vec4 _Padding;
};

void main() {
    v_texCoords = a_position;

    mat4 staticView = mat4(mat3(_ViewMatrix));
    vec4 position = _ProjectionMatrix * staticView * vec4(a_position, 1.0);

    gl_Position = position.xyww;
}

#type fragment
#version 450 core

out vec4 fragColor;
in vec3 v_texCoords;

uniform samplerCube _SkyboxTexture;

void main() {
    fragColor = texture(_SkyboxTexture, v_texCoords);
}