// GLSL tessellation control shader
#version 400

// Tessellation control output configuration
layout(vertices = 4) out;

// Uniform buffer object (also named "Constant Buffer")
layout(std140) uniform Settings
{
	mat4 wvpMatrix;
	float tessLevelInner;
	float tessLevelOuter;
	float twist;
	float _pad0;
};

// Input and output attributes
in vec3 vPosition[];
out vec3 tcPosition[];

// Tessellation control shader main function
void main()
{
	// Pass vertex input position to tessellation control output position
	tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
	
	// Write tessellation levels only once
	if (gl_InvocationID == 0)
	{
		// Set inner level
		gl_TessLevelInner[0] = tessLevelInner;
		gl_TessLevelInner[1] = tessLevelInner;
		
		// Set outer level (could also be individual for all quad edges)
		gl_TessLevelOuter[0] = tessLevelOuter;
		gl_TessLevelOuter[1] = tessLevelOuter;
		gl_TessLevelOuter[2] = tessLevelOuter;
		gl_TessLevelOuter[3] = tessLevelOuter;
	}
}
