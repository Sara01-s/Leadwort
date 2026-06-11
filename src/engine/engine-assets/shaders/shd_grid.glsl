#type vertex
#version 450 core

const vec2 verts[4] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 1.0, -1.0),
        vec2(-1.0,  1.0),
        vec2( 1.0,  1.0)
);

void main() {
    gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform CameraData {
    mat4 _ViewMatrix;
    mat4 _ProjectionMatrix;
    vec4 _CameraPosition;
    vec4 _Padding;
};

uniform mat4 _InvViewMatrix;
uniform mat4 _InvProjectionMatrix;
uniform vec2 _Resolution;

const float GRID_SIZE     = 1.0;
const float LINE_WIDTH    = 0.012;
const float GRID_OPACITY  = 0.19;
const float MAJOR_OPACITY = 0.1;

const vec3 GRID_COLOR   = vec3(0.6, 0.6, 0.6);
const vec3 X_AXIS_COLOR = vec3(0.8, 0.15, 0.15);
const vec3 Z_AXIS_COLOR = vec3(0.15, 0.45, 0.8);

float gridLine(float v, float width) {
    float fw   = fwidth(v);
    float dist = abs(fract(v - 0.5) - 0.5);
    return 1.0 - smoothstep(width - fw, width + fw, dist);
}

void main() {
    vec2 ndc = (gl_FragCoord.xy / _Resolution) * 2.0 - 1.0;

    vec4 nearView = _InvProjectionMatrix * vec4(ndc, -1.0, 1.0);
    vec4 farView  = _InvProjectionMatrix * vec4(ndc,  1.0, 1.0);
    nearView /= nearView.w;
    farView  /= farView.w;

    nearView.y = -nearView.y;
    nearView.z = -nearView.z;
    farView.y  = -farView.y;
    farView.z  = -farView.z;

    vec4 nearWorld = _InvViewMatrix * nearView;
    vec4 farWorld  = _InvViewMatrix * farView;

    vec3 rayOrigin = nearWorld.xyz;
    vec3 rayDir    = normalize(farWorld.xyz - nearWorld.xyz);

    if (abs(rayDir.y) < 1e-5) {
        discard;
    }

    float t = -rayOrigin.y / rayDir.y;

    if (t < 0.0) {
        discard;
    }

    vec3 worldPos = rayOrigin + t * rayDir;

    float thinGrid  = max(gridLine(worldPos.x / GRID_SIZE, LINE_WIDTH), gridLine(worldPos.z / GRID_SIZE, LINE_WIDTH));
    float majorGrid = max(gridLine(worldPos.x / (GRID_SIZE * 10.0), LINE_WIDTH * 1.5), gridLine(worldPos.z / (GRID_SIZE * 10.0), LINE_WIDTH * 1.5));

    float axisX = 1.0 - smoothstep(0.02, 0.04, abs(worldPos.z));
    float axisZ = 1.0 - smoothstep(0.02, 0.04, abs(worldPos.x));

    vec4 color = vec4(0.0);
    color = mix(color, vec4(GRID_COLOR, GRID_OPACITY),  thinGrid);
    color = mix(color, vec4(GRID_COLOR, MAJOR_OPACITY), majorGrid);
    color = mix(color, vec4(X_AXIS_COLOR, 1.0), axisX * 0.7);
    color = mix(color, vec4(Z_AXIS_COLOR, 1.0), axisZ * 0.7);

    color.a *= 1.0 - smoothstep(10.0, 500.0, length(worldPos.xz));

    if (color.a <= 0.001) {
        discard;
    }

    vec4 clip = _ProjectionMatrix * _ViewMatrix * vec4(worldPos, 1.0);
    gl_FragDepth = (clip.z / clip.w) * 0.5 + 0.5;

    fragColor = color;
}