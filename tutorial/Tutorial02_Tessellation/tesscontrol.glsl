// GLSL tessellation control shader
#version 400

// Tessellation control output configuration
layout(vertices = 4) out;

// Uniform buffer object (also named "Constant Buffer")
layout(std140) uniform Settings
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 worldMatrix;
	float tessLevelInner;
	float tessLevelOuter;
};

// Input and output attributes
in vec3 vPosition[];
out vec3 tcPosition[];
 
// Tessellation control shader main function
void main()
{
	tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
	if (gl_InvocationID == 0)
	{
		gl_TessLevelInner[0] = tessLevelInner;
		gl_TessLevelInner[1] = tessLevelInner;
		
		gl_TessLevelOuter[0] = tessLevelOuter;
		gl_TessLevelOuter[1] = tessLevelOuter;
		gl_TessLevelOuter[2] = tessLevelOuter;
		gl_TessLevelOuter[3] = tessLevelOuter;
	}
}
