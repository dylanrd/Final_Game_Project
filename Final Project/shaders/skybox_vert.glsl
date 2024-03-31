#version 450

layout (location = 0) in vec3 aPos;

out vec3 texCoords;

layout(location = 2) uniform mat4 projectionMatrix;
layout(location = 3) uniform mat4 viewMatrix;

void main()
{
    vec4 pos = projectionMatrix * viewMatrix * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
    texCoords = vec3(aPos.x, aPos.y, -aPos.z);
}  