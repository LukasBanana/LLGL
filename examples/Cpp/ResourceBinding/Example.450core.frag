// GLSL fragment shader

#version 450 core

layout(push_constant) uniform Model
{
    vec3 lightVec;
    uint instance; // DUMMY
};

layout(location = 0) in vec4 vWorldPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

layout(location = 0) out vec4 fColor;


// PIXEL SHADER

layout(binding = 4) uniform texture2D colorMap;
layout(binding = 5) uniform sampler colorMapSampler;

void main()
{
	// Sample color map
	vec4 color = texture(sampler2D(colorMap, colorMapSampler), vTexCoord);
    
	// Compute lighting
	vec3 normal = normalize(vNormal);
	float NdotL = max(0.2, dot(normal, lightVec));
	
	fColor = vec4(color.rgb * NdotL, color.a);
}

