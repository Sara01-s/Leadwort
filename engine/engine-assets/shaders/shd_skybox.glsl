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

    mat4 static_view = mat4(mat3(_ViewMatrix));
    vec4 clipPos = _ProjectionMatrix * static_view * vec4(a_position, 1.0);

    gl_Position = clipPos.xyww;
}

#type fragment
#version 450 core

out vec4 fragColor;
in vec3 v_texCoords;

uniform sampler2D _SkyboxTexture;

// (1 / TAU, 1 / PI)
const vec2 invAtan = vec2(0.15915494, 0.31830988);

vec2 sample_spherical_map(vec3 dir) {
    vec2 uv = vec2(atan(dir.z, dir.x), asin(dir.y));

    uv *= invAtan;
    uv += 0.5;
    uv.y = 1.0 - uv.y;

    return uv;
}

void main() {
    vec2 uv = sample_spherical_map(normalize(v_texCoords));
    vec3 color = texture(_SkyboxTexture, uv).rgb;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    fragColor = vec4(color, 1.0);
}