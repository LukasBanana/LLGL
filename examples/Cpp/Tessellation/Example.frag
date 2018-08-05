// GLSL fragment shader
#version 400

// Fragment input from the vertex shader
in vec3 teColor;

// Fragment output color
out vec4 fragColor;

// Fragment shader main function
void main()
{
	fragColor = vec4(teColor, 1);
}
