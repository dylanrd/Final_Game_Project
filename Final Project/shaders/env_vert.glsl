#version 450


layout(location = 0) uniform mat4 modelMatrix;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 projection;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 fragPosition;
out vec3 fragNormal;


void main()
{

    gl_Position = projection * view * vec4(position, 1.0);
    fragPosition = (modelMatrix * vec4(position, 1)).xyz;
    fragNormal = normal;
    
}