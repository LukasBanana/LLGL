// GLSL tessellation control shader
#version 450

// Tessellation control output configuration
layout(vertices = 4) out;

// Uniform buffer object (also named "Constant Buffer")
layout(std140, binding = 0) uniform Settings
{
	mat4 wvpMatrix;
	float tessLevelInner;
	float tessLevelOuter;
	float twist;
	float _pad0;
};

// Input and output attributes
layout(location = 0) in vec3 vPosition[];
layout(location = 0) out vec3 tcPosition[];

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
