#ifndef LW_PBR_HLSL
#define LW_PBR_HLSL

layout(binding = 14) uniform samplerCube _IBLIrradiance;
layout(binding = 13) uniform samplerCube _IBLPrefilter;
layout(binding = 12) uniform sampler2D   _IBLBrdfLUT;

const float MAX_PREFILTER_LOD = 4.0; // mipLevels - 1, must match IBLBaker::PrefilterEnvironment

uniform vec4  _Color;
uniform float _MetallicIntensity;
uniform float _RoughnessIntensity;

// Alpha test threshold, from the glTF alphaMode: MASK sets its cutoff here, OPAQUE and
// BLEND leave it at zero so nothing is ever discarded.
uniform float _AlphaCutoff;

#ifdef HAS_DIFFUSE
    uniform sampler2D _DiffuseTexture;
    uniform vec4 _DiffuseTexture_ST;
#endif
#ifdef HAS_SPECULAR
    uniform sampler2D _SpecularTexture;
    uniform vec4 _SpecularTexture_ST;
#endif
#if defined(HAS_NORMAL) && defined(HAS_TANGENTS)
    uniform sampler2D _NormalTexture;
    uniform vec4 _NormalTexture_ST;
#endif
#ifdef HAS_OPACITY
    uniform sampler2D _OpacityTexture;
    uniform vec4 _OpacityTexture_ST;
#endif
#ifdef HAS_EMISSIVE
    uniform sampler2D _EmissiveTexture;
    uniform vec4 _EmissiveTexture_ST;
#endif
#ifdef HAS_ROUGHNESS
    uniform sampler2D _RoughnessTexture;
    uniform vec4 _RoughnessTexture_ST;
#endif
#ifdef HAS_METALLIC
    uniform sampler2D _MetallicTexture;
    uniform vec4 _MetallicTexture_ST;
#endif
#ifdef HAS_AO
    uniform sampler2D _AmbientOcclusionTexture;
    uniform vec4 _AmbientOcclusionTexture_ST;
#endif

// ----------------------------------------------------------------
vec2 applyST(vec2 uv, vec4 st) {
    return uv * st.xy + st.zw;
}

const vec3 AMBIENT_LIGHT = vec3(0.03, 0.03, 0.04); // Unused when using IBL.

vec3 linearColorSpace(vec3 color) {
    return pow(color, vec3(2.2));
}

vec3 fresnelRoughness(vec3 F0, vec3 V, vec3 N, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - NdotV, 5.0);
}

vec3 sampleAlbedo(vec2 uv) {
    vec3 albedo = linearColorSpace(_Color.rgb);

    #ifdef HAS_DIFFUSE
        albedo *= linearColorSpace(texture(_DiffuseTexture, applyST(uv, _DiffuseTexture_ST)).rgb);
    #endif

    return albedo;
}


float sampleAlpha(vec2 uv) {
    float alpha = _Color.a;

    #if defined(HAS_DIFFUSE) && !defined(HAS_OPACITY)
        alpha *= texture(_DiffuseTexture, applyST(uv, _DiffuseTexture_ST)).a;
    #endif

    #ifdef HAS_OPACITY
        alpha *= texture(_OpacityTexture, applyST(uv, _OpacityTexture_ST)).a;
    #endif

    return alpha;
}

float sampleRoughness(vec2 uv) {
    float roughness = 0.5;

    #ifdef SPECULAR_GLOSSINESS
        #ifdef HAS_SPECULAR
            roughness = max(1.0 - texture(_SpecularTexture, applyST(uv, _SpecularTexture_ST)).a, 0.05);
        #endif
    #else
        roughness = max(_RoughnessIntensity, 0.05);

        #ifdef HAS_ROUGHNESS
            roughness *= texture(_RoughnessTexture, applyST(uv, _RoughnessTexture_ST)).g;
        #endif
    #endif

    return roughness;
}

float sampleMetallic(vec2 uv) {
    float metallic = 0.0;

    #ifndef SPECULAR_GLOSSINESS
        metallic = _MetallicIntensity;

        #ifdef HAS_METALLIC
            metallic *= texture(_MetallicTexture, applyST(uv, _MetallicTexture_ST)).b;
        #endif
    #endif

    return metallic;
}

vec3 sampleNormal(vec2 uv) {
    vec3 N = normalize(v_worldNormal);

    #if defined(HAS_NORMAL) && defined(HAS_TANGENTS)
        vec3 tangentNormal = texture(_NormalTexture, applyST(uv, _NormalTexture_ST)).xyz * 2.0 - 1.0;
        N = normalize(v_tbn * tangentNormal);
    #endif

    return N;
}

vec3 sampleEmission(vec2 uv) {
    vec3 emission = vec3(0.0);

    #ifdef HAS_EMISSIVE
        emission = texture(_EmissiveTexture, applyST(uv, _EmissiveTexture_ST)).rgb;
    #endif

    return emission;
}

vec3 sampleBaseReflectivity(vec2 uv, vec3 albedo, float metallic) {
    vec3 F0 = vec3(0.04);

    #ifdef SPECULAR_GLOSSINESS
        #ifdef HAS_SPECULAR
            F0 = linearColorSpace(texture(_SpecularTexture, applyST(uv, _SpecularTexture_ST)).rgb);
        #endif
    #else
        F0 = mix(vec3(0.04), albedo, metallic);

        #ifdef HAS_SPECULAR
            vec3 specularSample = linearColorSpace(texture(_SpecularTexture, applyST(uv, _SpecularTexture_ST)).rgb);
            F0 = mix(specularSample, albedo * specularSample, metallic);
        #endif
    #endif

    return F0;
}

float sampleAO(vec2 uv) {
    float ao = 1.0;

    #ifdef HAS_AO
        ao = texture(_AmbientOcclusionTexture, applyST(uv, _AmbientOcclusionTexture_ST)).r;
    #endif

    return ao;
}

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

vec3 PBR_Direct(
        vec3 F0,
        vec3 albedo,
        vec3 N, vec3 V, vec3 L, vec3 H,
        float roughness,
        float metallic,
        vec3 lightColor
) {
    vec3 Ks = fresnel(F0, V, H);
    vec3 Kd = 1.0 - Ks;
    vec3 KdMetallic = Kd * (1.0 - metallic);

    vec3 lambert = albedo / PI;
    vec3 specular = cookTorrance(roughness, F0, N, H, V, L);

    vec3 BRDF = KdMetallic * lambert + specular;

    return BRDF * lightColor * safe_dot(N, L);
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
    vec3 direct = PBR_Direct(F0, albedo, N, V, L, H, roughness, metallic, lightColor);

    vec3 ambientLight = ambient * albedo * ao;
    vec3 emissivity = emission + ambientLight;

    return emissivity + direct;
}

vec3 PBR_IBL(
    vec3 F0,
    vec3 albedo,
    vec3 N, vec3 V,
    float roughness,
    float metallic,
    float ao
) {
    vec3 Ks = fresnelRoughness(F0, V, N, roughness);
    vec3 Kd = (1.0 - Ks) * (1.0 - metallic);

    // Diffuse
    vec3 irradiance = texture(_IBLIrradiance, N).rgb;
    vec3 diffuseIBL = irradiance * albedo;

    // Specular
    vec3 R = reflect(-V, N);
    vec3 prefilteredColor = textureLod(_IBLPrefilter, R, roughness * MAX_PREFILTER_LOD).rgb;
    vec2 brdf = texture(_IBLBrdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specularIBL = prefilteredColor * (F0 * brdf.x + brdf.y);

    return (Kd * diffuseIBL + specularIBL) * ao;
}

/*
	Ks = Fresnel effect.
	Kd = 1 - Ks
	vec3 BRDF = Kd * f_diffuse + Ks * f_specular
	Kd + Ks max value should always be 1.
*/

#endif