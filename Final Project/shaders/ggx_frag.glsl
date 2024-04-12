#version 450 core

layout(std140) uniform Material // Must match the GPUMaterial defined in src/mesh.h
{
    vec3 kd;
	vec3 ks;
	float shininess;
	float transparency;
};

// Inputs from vertex shader
in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;
in mat3 TBN;

// Textures
layout(location = 4) uniform sampler2D albedoMap;
layout(location = 5) uniform sampler2D normalMap;
layout(location = 6) uniform sampler2D metallicMap;
layout(location = 7) uniform sampler2D roughnessMap;
layout(location = 8) uniform sampler2D aoMap;

// Light properties
layout(location = 9) uniform vec3 lightPos;
layout(location = 10) uniform vec3 lightColor;
layout(location = 11) uniform float lightIntensity;

// Camera properties
layout(location = 12) uniform vec3 camPos;

// Output color
out vec4 color;

// Function to compute GGX distribution
float GGXDistribution(float NdotH, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    return a2 / (3.14159265 * denom * denom);
}

// Geometry function using Smith's method
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}

// Geometry combining function
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// Fresnel function using Schlick's approximation
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
    vec3 albedo = texture(albedoMap, fragTexCoord).rgb;
    vec3 N = normalize(TBN * (texture(normalMap, fragTexCoord).rgb * 2.0 - 1.0));
    float metallic = texture(metallicMap, fragTexCoord).r;
    float roughness = texture(roughnessMap, fragTexCoord).r;
    float ao = texture(aoMap, fragTexCoord).r;

    vec3 V = normalize(camPos - fragPosition);
    vec3 L = normalize(lightPos - fragPosition);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float LdotH = max(dot(L, H), 0.0);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
    float D = GGXDistribution(NdotH, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    vec3 numerator = D * F * G;
    float denominator = 4.0 * NdotV * NdotL + 0.0001; // Avoid divide by zero
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = lightColor * lightIntensity;
    vec3 diffuse = kD * albedo / 3.14159265;

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 radiance = NdotL * irradiance * (diffuse + specular) + ambient;

    color = vec4(radiance, 1.0);
}
