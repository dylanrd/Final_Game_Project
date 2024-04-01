#version 450

layout(std140) uniform Material // Must match the GPUMaterial defined in src/mesh.h
{
    vec3 kd;
	vec3 ks;
	float shininess;
	float transparency;
};

layout(location = 3) uniform sampler2D colorMap;
layout(location = 6) uniform vec3 lightPos;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 ambient = 0.22 * kd; 
    const vec3 norm = normalize(fragNormal);

    vec3 lightDir = normalize(lightPos - fragPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * kd;

    vec3 viewDir = normalize(lightPos - fragPosition);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    vec3 specular = vec3(0.0);

    if (diff > 0.0) {
        float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
        specular = spec * ks;
    }

    vec3 col = kd * dot(norm, normalize(lightPos - fragPosition));
    
    vec3 result = (ambient + diffuse + specular) * col;

    
    fragColor = vec4(result, 1);
}
