/*
 * StreamOutput.410core.geom
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

layout(triangles) in;
layout(triangle_strip, max_vertices = 9) out;

layout(location = 0) in vec3 vNormal[];
layout(location = 1) in vec3 vColor[];

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 color;

void main()
{
    const vec3 instanceColors[3] = vec3[3]
	(
        vec3(0.8, 0.2, 0.1), // red
        vec3(0.2, 0.9, 0.2), // green
        vec3(0.2, 0.3, 0.9)  // blue
	);

    for (int instanceIndex = 0; instanceIndex < 3; ++instanceIndex)
    {
        for (int vertexIndex = 0; vertexIndex < 3; ++vertexIndex)
        {
            gl_Position = gsMatrices[instanceIndex] * gl_in[vertexIndex].gl_Position;
            normal    	= vNormal[vertexIndex];
            color      	= vColor[vertexIndex] * instanceColors[instanceIndex];
			EmitVertex();
        }
		EndPrimitive();
    }
}
