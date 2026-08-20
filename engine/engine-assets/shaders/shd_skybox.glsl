#type vertex
#version 450 core

layout(location = 0) in vec3 a_position;

layout (std140, binding = 0) uniform CameraData {
    mat4 _ViewMatrix;
    mat4 _ProjectionMatrix;
    vec4 _CameraPosition;
    vec4 _Padding;
};

out vec3 v_localPos;

void main() {
    v_localPos = a_position;

    mat4 viewNoTranslation = mat4(mat3(_ViewMatrix));

    vec4 pos = _ProjectionMatrix * viewNoTranslation * vec4(a_position, 1.0);
    gl_Position = pos.xyww;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 fragColor;

uniform sampler2D _SkyboxTexture;
uniform float _RotationDegrees;
uniform vec3  _Tint;
uniform float _Exposure;

in vec3 v_localPos;

const vec2 INV_ATAN = vec2(0.1591, 0.3183);
const float PI = 3.14159265359;

vec2 sampleSphericalMap(vec3 dir) {
    vec2 uv = vec2(atan(dir.z, dir.x), asin(dir.y));

    uv *= INV_ATAN;
    uv += 0.5;

    return uv;
}

vec3 rotateY(vec3 v, float radians) {
    float s = sin(radians);
    float c = cos(radians);

    return vec3(
        v.x * c + v.z * s,
        -v.y,
        -v.x * s + v.z * c
    );
}

void main() {
    vec3 dir = normalize(v_localPos);
    dir = rotateY(dir, radians(180));

    vec2 uv = sampleSphericalMap(dir);
    vec3 color = texture(_SkyboxTexture, uv).rgb;

    color *= _Tint * _Exposure;

    color *= 1.1;

    fragColor = vec4(color, 1.0);
}