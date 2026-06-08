#define PI 3.14159265358979323846

// GGX/Trowbridge-Reitz Normal Distribution
float D(float alpha, vec3 N, vec3 H) {
    float numerator = alpha * alpha;

    float NdotH = max(dot(N, H), 0.0);
    float denominator = PI * pow((NdotH * NdotH) * ((alpha * alpha) - 1.0) + 1.0, 2.0);
    denominator = max(denominator, 0.000001);

    return numerator / denominator;
}

// GGX/Schlick-Backmann Geometry Shadwing
float G1(float alpha, vec3 N, vec3 X) { // X is either the view or light vector.
    float numerator = max(dot(N, X), 0.0);

    float k = alpha / 2.0;
    float denominator = numerator * (1.0 - k) + k;
    denominator = max(denominator, 0.000001);

    return numerator / denominator;
}

// Smith Model
float G(float alpha, vec3 N, vec3 V, vec3 L) {
    return G1(alpha, N, V) * G1(alpha, N, L);
}

// Fresnel-schlick
vec3 F(vec3 F0, vec3 V, vec3 H) {
    return F0 + (vec3(1.0) - F0) * pow(1.0 - max(dot(V, H), 0.0), 5.0);
}

// Rendering equation for one light source
// N: Normal, V: View, L: Light, H: Halfway
vec3 PBR(
    vec3 F0,
    vec3 albedo,
    vec3 N, vec3 V, vec3 L, vec3 H,
    float roughness,
    float metallicIntensity,
    vec3 lightColor,
    vec3 emission,
    float ao,
    vec3 ambient
) {
    float alpha = roughness * roughness;

    // Fresnel
    vec3 Ks = F(F0, V, H);

    // Diffuse (reduced by metallicIntensitY)
    vec3 Kd = (vec3(1.0) - Ks) * (1.0 - metallicIntensity);
    vec3 lambert = albedo / PI;

    // Specular (Cook-Torrance)
    vec3 cookTorranceNumerator = D(alpha, N, H) * G(alpha, N, V, L) * F(F0, V, H);
    float cookTorranceDenominator = 4.0 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0);
    cookTorranceDenominator = max(cookTorranceDenominator, 0.000001);
    vec3 cookTorrance = cookTorranceNumerator / cookTorranceDenominator;

    vec3 BRDF = (Kd * lambert) + cookTorrance;
    float incomingLight = max(dot(L, N), 0.0);
    vec3 diffuseAndSpecular = (BRDF * incomingLight * lightColor);
    vec3 ambientColor = albedo * ambient * ao;
    vec3 outgoingLight = emission + ambientColor + diffuseAndSpecular;

    return outgoingLight;
}