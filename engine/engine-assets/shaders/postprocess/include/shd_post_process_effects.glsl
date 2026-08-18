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

vec3 applyChromaticAberration(sampler2D screenTex, vec2 uv, float offset) {
    float amount = -(offset / 100.0);

    vec3 color = vec3(0.0);
    color.r = texture(screenTex, vec2(uv.x + amount, uv.y)).r;
    color.g = texture(screenTex, uv).g;
    color.b = texture(screenTex, vec2(uv.x, uv.y + amount)).b;

    color *= (1.0 - amount * 0.5);

    return color;
}

vec3 applyExposure(vec3 color, float exposure) {
    return color * exp2(exposure);
}

vec3 applyBrightness(vec3 color, float brightness) {
    return color + vec3(brightness);
}

float linearizeDepth(float depth, float near, float far) {
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

// Bokeh helpers
float getCoC(float linearDepth, float focusDistance, float focusRange) {
    return clamp(abs(linearDepth - focusDistance) / max(focusRange, 0.001), 0.0, 1.0);
}

vec3 applyDepthOfField(
    sampler2D screenTex,
    sampler2D depthTex,
    vec2 uv,
    vec2 texelSize,
    float focusDistance,
    float focusRange,
    float maxBlurRadius,
    float nearPlane,
    float farPlane
) {
    float rawDepth = texture(depthTex, uv).r;
    float centerLinearDepth = linearizeDepth(rawDepth, nearPlane, farPlane);
    float centerCoC = getCoC(centerLinearDepth, focusDistance, focusRange);

    if (centerCoC < 0.01) {
        return texture(screenTex, uv).rgb;
    }

    vec3 colorAcc = vec3(0.0);
    float totalWeight = 0.0;
    float blurRadius = centerCoC * maxBlurRadius;

    const int SAMPLE_COUNT = 24;

    for (int i = 0; i < SAMPLE_COUNT; ++i) {
        float angle = (float(i) / float(SAMPLE_COUNT)) * 6.28318530718 * 3.0;
        float radiusT = sqrt(float(i) / float(SAMPLE_COUNT));
        vec2 sampleOffset = vec2(cos(angle), sin(angle)) * radiusT;

        vec2 offsetUV = uv + sampleOffset * blurRadius * texelSize;

        float sampleRawDepth = texture(depthTex, offsetUV).r;
        float sampleLinearDepth = linearizeDepth(sampleRawDepth, nearPlane, farPlane);
        float sampleCoC = getCoC(sampleLinearDepth, focusDistance, focusRange);

        float depthWeight = sampleLinearDepth <= centerLinearDepth
            ? 1.0
            : clamp(sampleCoC / max(centerCoC, 0.001), 0.0, 1.0);

        vec3 sampleColor = texture(screenTex, offsetUV).rgb;

        float weight = depthWeight;
        colorAcc += sampleColor * weight;
        totalWeight += weight;
    }

    vec3 blurredColor = totalWeight > 0.001 ? colorAcc / totalWeight : texture(screenTex, uv).rgb;

    vec3 originalColor = texture(screenTex, uv).rgb;
    return mix(originalColor, blurredColor, centerCoC);
}

// Bloom (single-pass approximation)
vec3 extractBrightness(vec3 color, float threshold) {
    float lum = max(color.r, max(color.g, color.b));
    float contribution = max(0.0, lum - threshold) / max(lum, 0.0001);

    return color * contribution;
}

vec3 applyBloom(
    sampler2D screenTex,
    vec2 uv,
    vec2 texelSize,
    float threshold,
    float intensity,
    float radius
) {
    vec3 bloomAcc = vec3(0.0);
    const int SAMPLE_COUNT = 16;

    for (int i = 0; i < SAMPLE_COUNT; ++i) {
        float angle = (float(i) / float(SAMPLE_COUNT)) * 6.28318530718 * 2.0;
        float radiusT = sqrt(float(i) / float(SAMPLE_COUNT));
        vec2 sampleOffset = vec2(cos(angle), sin(angle)) * radiusT;

        vec2 offsetUV = uv + sampleOffset * radius * texelSize;
        vec3 sampleColor = texture(screenTex, offsetUV).rgb;
        bloomAcc += extractBrightness(sampleColor, threshold);
    }

    bloomAcc /= float(SAMPLE_COUNT);
    return bloomAcc * intensity;
}