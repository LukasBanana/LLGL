/*
 * CommandBufferTier1.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_COMMAND_BUFFER_TIER1_H
#define LLGL_C99_COMMAND_BUFFER_TIER1_H


#include <LLGL-C/Export.h>
#include <LLGL-C/LLGLWrapper.h>
#include <stddef.h>


LLGL_C_EXPORT void llglDrawMesh(uint32_t numWorkGroupsX, uint32_t numWorkGroupsY, uint32_t numWorkGroupsZ);
LLGL_C_EXPORT void llglDrawMeshIndirect(LLGLBuffer buffer, uint64_t offset, uint32_t numCommands, uint32_t stride);
LLGL_C_EXPORT void llglDrawMeshIndirectExt(LLGLBuffer argumentsBuffer, uint64_t argumentsOffset, LLGLBuffer countBuffer, uint64_t countOffset, uint32_t maxNumCommands, uint32_t stride);


#endif



// ================================================================================
