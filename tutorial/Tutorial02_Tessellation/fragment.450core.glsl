// GLSL fragment shader
#version 450

// Fragment input from the vertex shader
layout(location = 0) in vec3 teColor;

// Fragment output color
layout(location = 0) out vec4 fragColor;

// Fragment shader main function
void main()
{
	fragColor = vec4(teColor, 1);
}
