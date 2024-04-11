#version 450

uniform samplerCube environmentMap;
layout(location = 3) uniform vec3 cameraPosition;
layout(location = 4) uniform float factor;
in vec3 fragPosition;
in vec3 fragNormal;

layout(location = 0) out vec4 fragColor;

void main()
{
    // Calculate reflection vector
    vec3 I = normalize(fragPosition - cameraPosition);
    vec3 R = reflect(I, normalize(fragNormal));
    // Sample cube map
    vec3 color = texture(environmentMap, R).rgb;
    // Output final color

    fragColor = mix(vec4(0), vec4(color, 1.0), factor);
}
