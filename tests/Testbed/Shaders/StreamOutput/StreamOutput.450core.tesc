/*
 * StreamOutput.450core.tesc
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

layout(std140, binding = 1) uniform SOScene
{
    mat4    vsMatrix;
    mat4    gsMatrices[3];
    vec4 	lightVec;
    float   normalizeFactorVS;
    float   normalizeFactorDS;
    float   tessLevelOuter;
    float   tessLevelInner;
};

layout(vertices = 3) out;

layout(location = 0) in vec3 vNormal[];
layout(location = 1) in vec3 vColor[];

layout(location = 0) out vec3 patchNormal[];
layout(location = 1) out vec3 patchColor[];

void main()
{
	// Pass through tess-control shader
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	patchNormal[gl_InvocationID] = vNormal[gl_InvocationID];
	patchColor[gl_InvocationID] = vColor[gl_InvocationID];
	
	// Write tessellation levels only once
	if (gl_InvocationID == 0)
	{
		gl_TessLevelOuter[0] = tessLevelOuter;
		gl_TessLevelOuter[1] = tessLevelOuter;
		gl_TessLevelOuter[2] = tessLevelOuter;

		gl_TessLevelInner[0] = tessLevelInner;
	}
}
