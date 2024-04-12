#version 450


layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 texCoord;
layout(location = 0) uniform mat4 mvpMatrix;

void main()
{
    // Transform vertex position from object space to clip space
    gl_Position = mvpMatrix * vec4(aPos, 1.0);
    
    // Pass texture coordinates to fragment shader
    texCoord = aTexCoord;
}