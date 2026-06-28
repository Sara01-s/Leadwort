#define PI 3.14159265358979323846

// Used to never have negative results.
// Since we're most dealing with light reflection calculations and negative light doesn't exists.
// Also negative results are outside the top hemisphere (omega) of the Rendering Equation integral.
float safe_dot(vec3 x, vec3 y) {
    return max(0.0, dot(x ,y));
}

// Used to never divide by 0.
float safe_divide(float x, float y) {
    return x / max(y, 0.000001);
}

vec3 safe_divide(vec3 x, float y) {
    return x / max(y, 0.000001);
}

// GGX/Trowbridge-Reitz Normal Distribution Function
// This function describes how the microfacets of the point we are on are distributed according to their roughness
float D(float roughness, vec3 N, vec3 H) {
    // a = alpha (the greek letter not opacity!)
    float a = roughness * roughness;

    float NdotH = safe_dot(N, H);
    float denominator = PI * pow((NdotH * NdotH) * ((a * a) - 1.0) + 1.0, 2.0);

    return safe_divide(a * a, denominator);
}

/*  Schlick-GGX Model: Geometry Shadowing Function
 *  this is a combination of the Smith and Schlick-beckmann model.
 *  the Smith model takes into account 2 types of geometrical shadowing interactions
 *      1. Geometry obstruction: The camera can't see a lit microfacet point because of the viewing angle.
 *      2. Geometry shadowing: The light ray isn't able to bounce to the camera.
 *  In this instance we'll use the Schlick-Beckmann function that does the same as the Smith Model, but uses
 *  the Light Vector instead of the View Vector.
*/
// X = V or L.
float G1(float roughness, vec3 N, vec3 X) {
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float NdotX = max(dot(N, X), 0.0001);
    float denom = NdotX * (1.0 - k) + k;

    return NdotX / max(denom, 0.0001);
}

// Smith Model
float G(float roughness, vec3 N, vec3 V, vec3 L) {
    return G1(roughness, N, V) * G1(roughness, N, L);
}

// Fresnel-schlick
vec3 fresnel(vec3 F0, vec3 V, vec3 H) {
    return F0 + (vec3(1.0) - F0) * pow(1.0 - max(dot(V, H), 0.0), 5.0);
}

// Specular
vec3 cookTorrance(float roughness, vec3 F0, vec3 N, vec3 H, vec3 V, vec3 L) {
    float NdotV = max(dot(V, N), 0.0001);
    float NdotL = max(dot(L, N), 0.0001);

    vec3  numerator   = D(roughness, N, H) * fresnel(F0, V, H) * G(roughness, N, V, L);
    float denominator = 4.0 * NdotV * NdotL;

    return safe_divide(numerator, denominator);
}

// Rendering equation for one light source
// F0 = base reflectivity, the reflectivity when the viewing angle is perpendicular to the surface.
// V = view vector.
// H = half-way vector. Vector between view and light vector.
vec3 PBR(
    vec3 F0,
    vec3 albedo,
    vec3 N, vec3 V, vec3 L, vec3 H,
    float roughness,
    float metallic,
    vec3 lightColor,
    vec3 emission,
    float ao,
    vec3 ambient
) {
    vec3 Ks = fresnel(F0, V, H);
    vec3 Kd = 1.0 - Ks;
    vec3 KdMetallic = Kd * (1.0 - metallic);

    vec3 lambert = albedo / PI;
    vec3 specular = cookTorrance(roughness, F0, N, H, V, L);

    // Ks is not used because is included in cookTorrance.
    vec3 BRDF = KdMetallic * lambert + specular;

    vec3 ambientLight = ambient * albedo * ao;
    vec3 emissivity = emission + ambientLight;

    // Rendering equation.
    vec3 outgoingLight = emissivity + BRDF * lightColor * safe_dot(N, L);

    return outgoingLight;
}

/*
	Ks = Fresnel effect.
	Kd = 1 - Ks
	vec3 BRDF = Kd * f_diffuse + Ks * f_specular
	Kd + Ks max value should always be 1.
*/
