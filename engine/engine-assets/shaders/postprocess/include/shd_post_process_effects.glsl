// Reinhard Tonemapping
vec3 applyReinhard(vec3 color, float exposure) {
    color *= exposure;
    return color / (color + vec3(1.0));
}

vec3 applyACES(vec3 color) {
    const float A = 2.51;
    const float B = 0.03;
    const float C = 2.43;
    const float D = 0.59;
    const float E = 0.14;

    return clamp((color * (A * color + B)) / (color * (C * color + D) + E), 0.0, 1.0);
}

float applyVignette(vec2 uv, float intensity) {
    vec2 dist = uv - 0.5;
    float vig = dot(dist, dist);

    return 1.0 - (vig * intensity);
}

vec3 applyGammaCorrection(vec3 color, float gamma) {
    return pow(color, vec3(1.0 / gamma));
}

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

vec3 applyFilmGrain(vec3 color, vec2 uv, float strength) {
    float grain = (random(uv) - 0.5) * strength;
    return color + grain;
}