/*
 * StreamOutput.450core.tese
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

#ifndef XFB_OUTPUT
#define XFB_OUTPUT 0
#endif

#if XFB_OUTPUT
layout(xfb_buffer = 0, xfb_stride = 40) out;
#endif

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

layout(triangles, fractional_odd_spacing, cw) in;

layout(location = 0) in vec3 patchNormal[];
layout(location = 1) in vec3 patchColor[];

#if XFB_OUTPUT
out gl_PerVertex
{
    layout(xfb_offset = 0) vec4 gl_Position;
};
layout(location = 0, xfb_offset = 16) out vec3 normal;
layout(location = 1, xfb_offset = 28) out vec3 color;
#else
layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 color;
#endif

invariant gl_Position;

void main()
{
	// Interpolate position
	vec4 tessPosition = gl_in[0].gl_Position * gl_TessCoord.x + gl_in[1].gl_Position * gl_TessCoord.y + gl_in[2].gl_Position * gl_TessCoord.z;
    vec4 normalizedPos = vec4(normalize(tessPosition.xyz), 1.0);
    gl_Position = mix(tessPosition, normalizedPos, normalizeFactorDS);
	
	// Interpolate normal
	vec3 tessNormal = patchNormal[0] * gl_TessCoord.x + patchNormal[1] * gl_TessCoord.y + patchNormal[2] * gl_TessCoord.z;
    normal = mix(tessNormal, normalizedPos.xyz, normalizeFactorDS);

    // Interpolate color
	color = patchColor[0] * gl_TessCoord.x + patchColor[1] * gl_TessCoord.y + patchColor[2] * gl_TessCoord.z;
}
