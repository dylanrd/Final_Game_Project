#version 450

in vec3 fragPos;
in vec3 worldNormal;  // Now explicitly called worldNormal fo
in vec2 TexCoords;
in mat3 TBN;  // TBN matrix passed from vertex shader

layout(location = 3) uniform sampler2D albedoMap;
layout(location = 4) uniform sampler2D normalMap;
layout(location = 5) uniform sampler2D metallicMap;
layout(location = 6) uniform sampler2D roughnessMap;
layout(location = 7) uniform sampler2D aoMap;

layout(location = 8) uniform vec3 lightPos;
layout(location = 9) uniform vec3 lightColor;
layout(location = 10) uniform float lightIntensity;
layout(location = 11) uniform vec3 camPos;

out vec4 color;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 CalculateDirectLight(vec3 N, vec3 V, vec3 L, vec3 albedo, float metallic, float roughness, float ao);

void main() {
    vec3 albedo = texture(albedoMap, TexCoords).rgb;
    float metallic = texture(metallicMap, TexCoords).r;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ao = texture(aoMap, TexCoords).r;

    vec3 normalFromMap = texture(normalMap, TexCoords).rgb;
    normalFromMap = normalFromMap * 2.0 - 1.0; // Transform from [0, 1] to [-1, 1]
    vec3 N = normalize(TBN * normalFromMap);  // Transform normal from tangent to world space

    vec3 V = normalize(camPos - fragPos);
    vec3 L = normalize(lightPos - fragPos);

    vec3 lighting = CalculateDirectLight(N, V, L, albedo, metallic, roughness, ao);
    color = vec4(lighting, 1.0);
}


// Implementation of the utility functions used above

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159265359 * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 CalculateDirectLight(vec3 N, vec3 V, vec3 L, vec3 albedo, float metallic, float roughness, float ao) {
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 H = normalize(V + L);
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = lightColor * lightIntensity * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
   
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular = numerator / max(denominator, 0.001);  
    
    // Contribution from each component
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;  
    
    float NdotL = max(dot(N, L), 0.0);        
    vec3 diffuse = (albedo / 3.14159265359) * NdotL;
    
    return (kD * diffuse + specular) * radiance * ao;
}
