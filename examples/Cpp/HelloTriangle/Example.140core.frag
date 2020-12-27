// GLSL shader version 1.40 (for OpenGL 3.1 Core Profile)
#version 140

// Fragment input from the vertex shader
in vec4 vertexColor;

// Fragment output color
out vec4 fragColor;

// Fragment shader main function
void main()
{
	fragColor = vertexColor;
}
