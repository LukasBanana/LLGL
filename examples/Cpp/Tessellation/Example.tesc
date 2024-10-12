// GLSL tessellation control shader
#version 400 core

// Tessellation control output configuration
layout(vertices = 4) out;

// Input constant data
layout(std140) uniform Scene
{
    mat4    vpMatrix;
    mat4    vMatrix;
    mat4    wMatrix;
    vec3    lightVec;
    float   texScale;
    float   tessLevelInner;
    float   tessLevelOuter;
    float   maxHeightFactor;
    float   shininessPower;
};

// Input and output attributes
in vec3 vWorldPos[];
in vec3 vNormal[];
in vec3 vTangent[];
in vec3 vBitangent[];
in vec2 vTexCoord[];

out vec3 tcWorldPos[];
out vec3 tcNormal[];
out vec3 tcTangent[];
out vec3 tcBitangent[];
out vec2 tcTexCoord[];

// Tessellation-control shader main function
void main()
{
	// Pass vertex input from tessellation-control to tessellation-evaluation stage
	tcWorldPos[gl_InvocationID] 	= vWorldPos[gl_InvocationID];
	tcNormal[gl_InvocationID] 		= vNormal[gl_InvocationID];
	tcTangent[gl_InvocationID] 		= vTangent[gl_InvocationID];
	tcBitangent[gl_InvocationID] 	= vBitangent[gl_InvocationID];
	tcTexCoord[gl_InvocationID] 	= vTexCoord[gl_InvocationID];
	
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
