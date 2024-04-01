#version 450

layout(location = 0) uniform mat4 mvpMatrix;
layout(location = 1) uniform vec3 cameraPosition; // Camera position
layout(location = 2) uniform mat4 modelMatrix;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoord;

void main()
{
    vec3 positionRelativeToCamera = position - cameraPosition;
    
    // Apply model, view, and projection transformations to the vertex position
    gl_Position = mvpMatrix * vec4(positionRelativeToCamera, 1.0);
    
    fragPosition = (modelMatrix * vec4(position, 1)).xyz;
    fragNormal = normal;
    fragTexCoord = texCoord;
}
