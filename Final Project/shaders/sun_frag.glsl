#version 450

layout(std140) uniform Material // Must match the GPUMaterial defined in src/mesh.h
{
    vec3 kd;
	vec3 ks;
	float shininess;
	float transparency;
};


layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(kd, 1);
}
