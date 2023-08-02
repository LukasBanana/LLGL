/*
 * ResourceViewHeapFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_RESOURCE_VIEW_HEAP_FLAGS_H
#define LLGL_C99_RESOURCE_VIEW_HEAP_FLAGS_H


#include <LLGL-C/TextureFlags.h>
#include <LLGL-C/BufferFlags.h>
#include <stdint.h>


/* ----- Flags ----- */

enum LLGLBarrierFlags
{
    LLGLBarrierStorageBuffer    = (1 << 0),
    LLGLBarrierStorageTexture   = (1 << 1),
    LLGLBarrierStorage          = (LLGLBarrierStorageBuffer | LLGLBarrierStorageTexture),
};


/* ----- Structures ----- */

typedef struct LLGLResourceViewDescriptor
{
    LLGLResource                resource;
    LLGLTextureViewDescriptor   textureView;
    LLGLBufferViewDescriptor    bufferView;
    uint32_t                    initialCount;
}
LLGLResourceViewDescriptor;

typedef struct LLGLResourceHeapDescriptor
{
    LLGLPipelineLayout  pipelineLayout;
    uint32_t            numResourceViews;
    long                barrierFlags;
}
LLGLResourceHeapDescriptor;


#endif



// ================================================================================
