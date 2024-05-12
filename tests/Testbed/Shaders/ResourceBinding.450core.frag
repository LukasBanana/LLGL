/*
 * ResourceBinding.450core.frag
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

#ifndef ENABLE_SPIRV
#define ENABLE_SPIRV 0
#endif

#if ENABLE_SPIRV
#extension  GL_EXT_samplerless_texture_functions : enable
#endif

layout(binding = 0, std430) readonly buffer inBufferA
{
    ivec4 inBufferA_arr[];
};
layout(binding = 1, std430) readonly buffer inBufferB
{
    ivec4 inBufferB_arr[];
};
layout(binding = 4, std430) writeonly buffer outBufferB
{
    ivec4 outBufferB_arr[];
};

#if ENABLE_SPIRV
layout(binding = 5) uniform itexture1D inTextureA;
layout(binding = 6) uniform itexture1D inTextureB;
#else
layout(binding = 2) uniform isampler1D inTextureA;
layout(binding = 4) uniform isampler1D inTextureB;
#endif

layout(location = 0) in vec2 vTexCoord;

layout(location = 0) out ivec4 outColorA;
layout(location = 1) out ivec4 outColorB;

void main()
{
    int id = int(vTexCoord.x);

    outBufferB_arr[id] = (inBufferA_arr[id] - inBufferB_arr[id]) / 2;

    outColorA = (texelFetch(inTextureA, id, 0) + texelFetch(inTextureB, id, 0));
    outColorB = (texelFetch(inTextureA, id, 0) - texelFetch(inTextureB, id, 0)) * 2;
}