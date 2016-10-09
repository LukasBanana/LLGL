// GLSL fragment shader
#version 140

uniform sampler2DArray tex;

// Fragment input from the vertex shader
in vec3 vTexCoord;
in vec3 vColor;

// Fragment output color
out vec4 fragColor;

// Fragment shader main function
void main()
{
	// Sample texture color
	vec4 color = texture(tex, vTexCoord);
	
	if (color.a < 0.5)
		discard;
	
	color.rgb *= vColor;
	
	fragColor = color;
}
