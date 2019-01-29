// GLSL shader version 1.20 (for OpenGL 2.x)
#version 120

// Fragment input from the vertex shader
varying vec3 vertexColor;

// Fragment shader main function
void main()
{
	gl_FragColor = vec4(vertexColor, 1);
}
