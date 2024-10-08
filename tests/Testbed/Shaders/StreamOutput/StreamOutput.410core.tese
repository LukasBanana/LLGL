/*
 * StreamOutput.410core.tese
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 410 core

layout(std140) uniform SOScene
{
    mat4    vsMatrix;
    mat4    gsMatrices[3];
    vec4 	lightVec;
    float   normalizeFactorVS;
    float   normalizeFactorDS;
    float   tessLevelOuter;
    float   tessLevelInner;
};

layout(triangles, fractional_odd_spacing, ccw) in; // <-- CCW for OpenGL because projection works differently to Direct3D

layout(location = 0) in vec3 patchNormal[];
layout(location = 1) in vec3 patchColor[];

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 color;

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
