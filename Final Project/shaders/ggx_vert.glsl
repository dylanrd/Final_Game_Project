#version 450

layout(location = 0) uniform mat4 mvpMatrix;
layout(location = 1) uniform mat4 modelMatrix;
// Normals should be transformed differently than positions:
// https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
layout(location = 2) uniform mat3 normalModelMatrix;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 tangent;


out vec3 fragPos;
out vec3 fragNormal;
out vec2 TexCoords;
out mat3 TBN;

void main()
{
    gl_Position = mvpMatrix * vec4(position, 1);
    
    fragPos = (modelMatrix * vec4(position, 1)).xyz;

    TexCoords = texCoord;
    // Calculate the Normal, Tangent, and Bitangent vectors
    vec3 N = normalize(normalModelMatrix * normal);
    vec3 T = normalize(vec3(modelMatrix * vec4(tangent, 0.0)));
    vec3 B = cross(N, T) * sign(tangent.z);  // Calculate Bitangent, includes correction for handedness

    // Construct the TBN matrix
    TBN = mat3(T, B, N);

}
