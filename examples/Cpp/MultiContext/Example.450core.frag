// GLSL shader version 4.50 (for Vulkan)
#version 450

// Fragment input from the geometry shader
layout(location = 0) in vec3 geometryColor;

// Fragment output color
layout(location = 0) out vec4 fragColor;

// Fragment shader main function
void main()
{
	fragColor = vec4(geometryColor, 1);
}
