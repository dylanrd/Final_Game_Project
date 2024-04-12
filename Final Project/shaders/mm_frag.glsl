#version 450

in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D textureMap; 

void main()
{
    // Sample texture using texture coordinates
    vec4 textureColor = texture(textureMap, texCoord);
    
    // Output texture color
    FragColor = textureColor;
}