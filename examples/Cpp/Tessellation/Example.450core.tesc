// GLSL tessellation control shader
#version 450 core

// Tessellation control output configuration
layout(vertices = 4) out;

// Input constant data
layout(std140, binding = 1) uniform Scene
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
layout(location = 0) in vec3 vWorldPos[];
layout(location = 1) in vec3 vNormal[];
layout(location = 2) in vec3 vTangent[];
layout(location = 3) in vec3 vBitangent[];
layout(location = 4) in vec2 vTexCoord[];

layout(location = 0) out vec3 tcWorldPos[];
layout(location = 1) out vec3 tcNormal[];
layout(location = 2) out vec3 tcTangent[];
layout(location = 3) out vec3 tcBitangent[];
layout(location = 4) out vec2 tcTexCoord[];

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
