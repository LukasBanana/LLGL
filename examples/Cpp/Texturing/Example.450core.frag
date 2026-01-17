// GLSL texturing fragment shader

#version 450

layout(std140, binding = 1) uniform Scene
{
	mat4 wvpMatrix;
	mat4 wMatrix;
	vec3 lightVec;
};

layout(binding = 2) uniform texture2D colorMap;
layout(binding = 3) uniform sampler samplerState;

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec2 vTexCoord;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec4 color = texture(sampler2D(colorMap, samplerState), vTexCoord);
    
	// Apply lambert factor for simple shading
	float NdotL = dot(lightVec, normalize(vNormal));
	color.rgb *= mix(0.2, 1.0, NdotL);
    
    fragColor = color;
}
