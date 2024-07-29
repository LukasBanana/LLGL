// GLSL fragment shader

#version 430 core

#ifdef GL_ES
precision mediump float;
precision mediump sampler2D;
#endif

uniform vec3 lightVec;

layout(location = 0) in vec4 vWorldPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

out vec4 fColor;


// PIXEL SHADER

uniform sampler2D colorMap;

void main()
{
	// Sample color map
	vec4 color = texture(colorMap, vTexCoord);

	// Compute lighting
	vec3 normal = normalize(vNormal);
	float NdotL = max(0.2, dot(normal, lightVec));
	
	fColor = vec4(color.rgb * NdotL, color.a);
}

