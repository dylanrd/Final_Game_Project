#version 450

layout(std140) uniform Material // Must match the GPUMaterial defined in src/mesh.h
{
    vec3 kd;
	vec3 ks;
	float shininess;
	float transparency;
};

layout(location = 3) uniform sampler2D colorMap;


layout(location = 6) uniform vec3 lightPos[3]; //positions
layout(location = 11) uniform vec3 lightAtt[3]; //attenuations

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 ambient = 0.22 * kd; 
    const vec3 normal = normalize(fragNormal);

    vec3 accDiff = vec3(0.0);
    vec3 accSpec = vec3(0.0);
    vec3 accCol = vec3(0.0);

   
    for (int i = 0; i < 3; i++) {

        float distance = length(lightPos[i] - fragPosition);
        
        float attenuationFactor = lightAtt[i].x + (lightAtt[i].y * distance) + (lightAtt[i].z * distance * distance);
        
        vec3 lightDir = normalize(lightPos[i] - fragPosition);
        float diff = max(dot(normal, lightDir), 0.0);
        accDiff += (diff * kd)/attenuationFactor;

        vec3 viewDir = normalize(lightPos[i] - fragPosition);
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        

        if (diff > 0.0) {
            float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
            accSpec += (spec * ks)/ attenuationFactor;
        }
  
        
        //accCol = accCol + (kd * dot(norm, normalize(lightPos[i] - fragPosition)))/ attenuationFactor;
    }
    vec3 result = (accDiff + accSpec) ;

    vec3 transparentColor = vec3(1.0, 1.0, 1.0); // Assuming white is fully transparent
    vec3 finalColor = mix(transparentColor, result, transparency);

    // Final color calculation
    fragColor = vec4(finalColor, 1);
}
