/*
 * ResourceBinding.450core.vert
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

#ifndef ENABLE_SPIRV
#define ENABLE_SPIRV 0
#endif

layout(binding = 0, std430) readonly buffer inBufferA
{
    ivec4 inBufferA_arr[];
};
layout(binding = 1, std430) readonly buffer inBufferB
{
    ivec4 inBufferB_arr[];
};
layout(binding = 2, std430) writeonly buffer outBufferA
{
    ivec4 outBufferA_arr[];
};

layout(location = 0) out vec2 vTexCoord;

#if ENABLE_SPIRV
out gl_PerVertex
{
    vec4 gl_Position;
};
#endif

void main()
{
    #if ENABLE_SPIRV
    int id = int(gl_VertexIndex);
    #else
    int id = int(gl_VertexID);
    #endif

	gl_Position = vec4(0.0, 0.0, 0.0, 1.0); //vec4(id == 2 ? 3.0 : -1.0, id == 0 ? 3.0 : -1.0, 1.0, 1.0);
	vTexCoord = vec2(0.0, 0.0); //gl_Position.xy * 0.5 + 0.5;

    outBufferA_arr[id] = (inBufferA_arr[id] + inBufferB_arr[id]) * 3;
}
