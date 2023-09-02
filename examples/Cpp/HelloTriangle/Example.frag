// GLSL shader version 1.30 (for OpenGL 3.1)
#version 130

#ifdef GL_ES
precision mediump float;
#endif

// Fragment input from the vertex shader
in vec3 vertexColor;

// Fragment output color
out vec4 fragColor;

// Fragment shader main function
void main()
{
	fragColor = vec4(vertexColor, 1);
}
