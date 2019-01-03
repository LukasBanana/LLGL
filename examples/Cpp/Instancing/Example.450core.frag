// GLSL fragment shader
#version 450 core

layout(binding = 3) uniform texture2DArray tex;
layout(binding = 4) uniform sampler texSampler;

// Fragment input from the vertex shader
layout(location = 0) in vec3 vTexCoord;
layout(location = 1) in vec3 vColor;

// Fragment output color
layout(location = 0) out vec4 fragColor;

// Fragment shader main function
void main()
{
	// Sample texture color
	vec4 color = texture(sampler2DArray(tex, texSampler), vTexCoord);
	
	if (color.a < 0.5)
		discard;
	
	color.rgb *= vColor;
	
	fragColor = color;
}
