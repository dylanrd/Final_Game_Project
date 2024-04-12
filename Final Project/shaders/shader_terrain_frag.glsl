
#version 450

layout(std140) uniform Material // Must match the GPUMaterial defined in src/mesh.h
{
    vec3 kd;
	vec3 ks;
	float shininess;
	float transparency;
};

//layout (std140, binding = 8) uniform MyUniformBlock {
//    vec3 myArray[2]; // This must match the structure defined in the application
//};

//layout (std140, binding = 9) uniform MyUniformBlock2 {
//    vec3 myArray2[2]; // This must match the structure defined in the application
//};

layout(location = 3) uniform sampler2D colorMap;
layout(location = 4) uniform sampler2D normalMap;
layout(location = 5) uniform bool useMaterial;

layout(location = 11) uniform vec3 myArray2[2]; //attenuations

layout(location = 7) uniform vec3 myArray[2]; //positions



in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;
in vec3 fragTangent;

layout(location = 0) out vec4 fragColor;

void main()
{
    
   

    vec3 Normal = normalize(fragNormal);
    vec3 Tangent = normalize(fragTangent);
    Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    vec3 Bitangent = cross(Tangent, Normal);
    vec3 BumpMapNormal = texture(normalMap, fragTexCoord).rgb;
    BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0, 1.0, 1.0);
    vec3 NewNormal;
    mat3 TBN = mat3(Tangent, Bitangent, Normal);
    NewNormal = TBN * BumpMapNormal;
    NewNormal = normalize(NewNormal);
    
    vec3 ambient = 0.22 * kd; 
    const vec3 norm = normalize(NewNormal);

    vec3 accDiff = vec3(0.0);
    vec3 accSpec = vec3(0.0);
    vec3 accCol = vec3(0.0);
 

    for (int i = 0; i < 2; i++) {

        float distance = length(myArray[i] - fragPosition);
        
        float attenuationFactor = myArray2[i].x + (myArray2[i].y * distance) + (myArray2[i].z * distance * distance);
        
        vec3 lightDir = normalize(myArray[i] - fragPosition);
        float diff = max(dot(norm, lightDir), 0.0);
        accDiff = accDiff + (diff * kd)/attenuationFactor;

        vec3 viewDir = normalize(myArray[i] - fragPosition);
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        

        if (diff > 0.0) {
            float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
            accSpec = accSpec + (spec * ks)/attenuationFactor;
        }

        //accCol = accCol + (kd * dot(norm, normalize(myArray[i] - fragPosition)))/ attenuationFactor;
    }
    
    
    vec3 result = (accDiff + accSpec);

    vec3 transparentColor = vec3(1.0, 1.0, 1.0); // Assuming white is fully transparent
    vec3 finalColor = mix(transparentColor, result, transparency);
    
    //vec3 col = kd * dot(normalize(NewNormal), TBN * normalize(fragPosition - lightPos));
    
    
    fragColor = vec4(result * texture(colorMap, fragTexCoord).rgb, 1);
}