// GLSL texturing shader
#version 140

uniform sampler2D colorMap;

// Fragment input from the vertex shader
in vec2 vTexCoord;

// Fragment output color
out vec4 fragColor;

// Fragment shader main function
void main()
{
	fragColor = texture(colorMap, vTexCoord);
}
