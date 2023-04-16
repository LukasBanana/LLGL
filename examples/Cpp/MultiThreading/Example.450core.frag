// GLSL fragment shader
#version 450 core

// Fragment input from the vertex shader
layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec3 vColor;

// Fragment output color
layout(location = 0) out vec4 fragColor;

// Fragment shader main function
void main()
{
	vec4 color = vec4(vColor, 1);
    
	// Apply lambert factor for simple shading
	const vec3 lightVec = vec3(0, 0, -1);
	float NdotL = dot(lightVec, normalize(vNormal));
	color.rgb *= mix(0.2, 1.0, NdotL);
    
    fragColor = color;
}
