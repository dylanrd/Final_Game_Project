#version 450

layout(std140) uniform Material // Must match the GPUMaterial defined in src/mesh.h
{
    vec3 kd;
	vec3 ks;
	float shininess;
	float transparency;
};

layout(location = 3) uniform sampler2D colorMap;
layout(location = 4) uniform sampler2D normalMap;
layout(location = 5) uniform bool useMaterial;
layout(location = 6) uniform vec3 lightPos;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;
in vec3 fragTangent;

layout(location = 0) out vec4 fragColor;

void main()
{
    
    vec3 Normal = normalize(fragNormal);
    vec3 Tangent = normalize(fragTangent);
    //Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    vec3 Bitangent = cross(Tangent, Normal);
    vec3 BumpMapNormal = texture(normalMap, fragTexCoord).rgb;
    BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0, 1.0, 1.0);
    vec3 NewNormal;
    mat3 TBN = mat3(Tangent, Bitangent, Normal);
    NewNormal = TBN * BumpMapNormal;
    NewNormal = normalize(NewNormal);
    
    
    vec3 col = kd * dot(normalize(NewNormal), TBN * normalize(lightPos - fragPosition));
    
    
    fragColor = vec4(col * texture(colorMap, fragTexCoord).rgb, 1);
}