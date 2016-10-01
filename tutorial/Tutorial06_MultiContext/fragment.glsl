// GLSL shader version 1.50 (for OpenGL 3.2)
#version 150

// Fragment input from the geometry shader
in vec3 geometryColor;

// Fragment output color
out vec4 fragColor;

// Fragment shader main function
void main()
{
	fragColor = vec4(geometryColor, 1);
}
