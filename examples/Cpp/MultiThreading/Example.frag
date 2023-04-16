// GLSL fragment shader
#version 410

// Fragment input from the vertex shader
in vec3 vNormal;
in vec3 vColor;

// Fragment output color
out vec4 fragColor;

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
