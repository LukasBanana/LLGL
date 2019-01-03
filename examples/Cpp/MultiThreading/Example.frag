// GLSL fragment shader
#version 410

// Fragment input from the vertex shader
in vec3 vertexColor;

// Fragment output color
out vec4 fragColor;

// Fragment shader main function
void main()
{
	fragColor = vec4(vertexColor, 1);
}
