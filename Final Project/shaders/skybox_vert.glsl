#version 450

layout(location = 0) uniform mat4 mvpMatrix;
layout(location = 1) uniform mat4 modelMatrix;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoord;

void main()
{
    gl_Position = mvpMatrix * vec4(position, 1);
    
    fragPosition = (modelMatrix * vec4(position, 1)).xyz;
    fragNormal = normal;
    fragTexCoord = texCoord;
}
