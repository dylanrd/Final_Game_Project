#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;
layout(location = 3) in vec3 tangent;

layout(location = 0)uniform mat4 model;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 projection;

out vec3 FragPos;
out vec3 WorldNormal;
out vec2 TexCoords;
out mat3 TBN;

void main() {
    FragPos = vec3(model * vec4(position, 1.0));
    WorldNormal = normalize(mat3(transpose(inverse(model))) * normal);
    vec3 T = normalize(mat3(model) * tangent);
    vec3 B = cross(WorldNormal, T);
    TBN = mat3(T, B, WorldNormal);

    gl_Position = projection * view * vec4(FragPos, 1.0);
    TexCoords = texCoords;
}