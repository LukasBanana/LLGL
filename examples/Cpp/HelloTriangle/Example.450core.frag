// GLSL shader version 4.50 (for Vulkan)
#version 450 core

// Fragment input from the vertex shader
layout(location = 0) in vec3 vertexColor;

// Fragment output color
layout(location = 0) out vec4 fragColor;

// Fragment shader main function
void main()
{
	fragColor = vec4(vertexColor, 1);
}
