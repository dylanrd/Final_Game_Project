#version 450

layout(std140) uniform Material // Must match the GPUMaterial defined in src/mesh.h
{
    vec3 kd;
	vec3 ks;
	float shininess;
	float transparency;
};

layout(location = 3) uniform sampler2D colorMap;
layout(location = 4) uniform bool hasTexCoords;
layout(location = 5) uniform bool useMaterial;
layout(location = 6) uniform vec3 lightPos;
layout(location = 7) uniform float factor;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

layout(location = 0) out vec4 fragColor;


void main()
{
    vec4 skyColor = vec4(texture(colorMap, fragTexCoord).rgb, 1);
	
	fragColor = mix(vec4(0.0f), skyColor, factor);
    
}
