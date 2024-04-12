#version 450 core

// Inputs from vertex shader
in vec3 fragPos;
in vec3 fragNormal;
in vec2 TexCoords;
in mat3 TBN;

// Textures
layout(location = 3) uniform sampler2D albedoMap;
layout(location = 4) uniform sampler2D normalMap;
layout(location = 5) uniform sampler2D metallicMap;
layout(location = 6) uniform sampler2D roughnessMap;
layout(location = 7) uniform sampler2D aoMap;

// Light properties
layout(location = 8) uniform vec3 lightPos;
layout(location = 9) uniform vec3 lightColor;
layout(location = 10) uniform float lightIntensity;

// Camera properties
layout(location = 11) uniform vec3 camPos;

// Output color
out vec4 color;

// Constants
const float pi = 3.14159265359;

// Function prototypes
vec3 getNormalFromMap();
float distributionGGX(vec3 N, vec3 H, float roughness);
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

void main() {
    // Extract data from maps
    vec3 albedo = texture(albedoMap, TexCoords).rgb;
    float metallic = texture(metallicMap, TexCoords).r;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ao = texture(aoMap, TexCoords).r;

    // Get normals from map and blend with vertex normals
    vec3 texNormal = getNormalFromMap();
    vec3 normal = normalize(mix(fragNormal, texNormal, 0.5));  // Ensure the blend is correct

    // Correct normal orientation if inverted (add a check or manual inversion if necessary)
    if (dot(normal, fragNormal) < 0.0) {
        normal = -normal;  // Invert the normal if it's flipped
    }

    vec3 V = normalize(camPos - fragPos);
    vec3 L = normalize(lightPos - fragPos);
    vec3 H = normalize(V + L);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    float NdotV = max(dot(normal, V), 0.0);

    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    float D = distributionGGX(normal, H, roughness);
    float G = geometrySmith(normal, V, L, roughness);

    vec3 nominator = F * G * D;
    float denominator = max(4.0 * NdotV * max(dot(normal, L), 0.0), 0.001);
    vec3 specular = nominator / denominator;

    float attenuation = 1.0f; //lightIntensity / (length(lightPos - fragPos) * length(lightPos - fragPos));
    vec3 radiance = lightColor * attenuation;

    // Correct diffuse component
    vec3 diffuse = (1.0 - max(dot(F, vec3(0.04)), 0.0)) * albedo * max(dot(normal, L), 0.0) * radiance;
    vec3 ambient = ao * albedo * vec3(0.03);

    color = vec4(diffuse + specular + ambient, 1.0);
}


// Functions
vec3 getNormalFromMap() {
    vec3 normal = texture(normalMap, TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);  // Unpack the normal
    return normalize(TBN * normal);
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    return a2 / (pi * denom * denom);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.01);
    float NdotL = max(dot(N, L), 0.01);
    float ggxV = NdotV * (1.0 - roughness) + roughness;
    float ggxL = NdotL * (1.0 - roughness) + roughness;
    return 1.0 / (ggxV * ggxL);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
