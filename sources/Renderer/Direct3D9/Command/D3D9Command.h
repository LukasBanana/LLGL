/*
 * D3D9Command.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_COMMAND_H
#define LLGL_D3D9_COMMAND_H


#include <LLGL/IndirectArguments.h>
#include <cstddef>
#include <cstdint>
#include "../Direct3D9.h"


namespace LLGL
{


class D3D9Buffer;
class D3D9EmulatedSampler;
class D3D9ConstantsCache;
class D3D9PipelineState;
class D3D9CommandBuffer;

struct D3D9CmdExecute
{
    const D3D9VirtualCommandBuffer* vcmdBuffer;
};

struct D3D9CmdClear
{
    DWORD       flags;
    D3DCOLOR    color;
    float       z;
    DWORD       stencil;
};

struct D3D9CmdSetRenderTargets
{
    UINT                count;
    IDirect3DSurface9*  depthStencilSurface;
//  IDirect3DSurface9*  targets[count];
};

struct D3D9CmdSetViewport
{
    D3DVIEWPORT9 viewport;
};

struct D3D9CmdSetScissorRect
{
    RECT scissorRect;
};

struct D3D9CmdSetIndices
{
    IDirect3DIndexBuffer9* indexBuffer;
};

struct D3D9CmdSetStreamSource
{
    UINT                    stream;
    IDirect3DVertexBuffer9* vertexBuffer;
    UINT                    offset;
    UINT                    stride;
};

struct D3D9CmdSetPipelineState
{
    D3D9PipelineState* pipelineState;
};

struct D3D9CmdSetRenderStates
{
    UINT                    numRenderStates;
    struct D3DRenderState
    {
        D3DRENDERSTATETYPE  type;
        DWORD               value;
    };
//  D3DRenderState          renderStates[numRenderStates];
};

struct D3D9CmdBindTexture
{
    DWORD                   stage;
    IDirect3DBaseTexture9*  texture;
};

struct D3D9CmdBindSampler
{
    DWORD                       stage;
    const D3D9EmulatedSampler*  emulatedSampler;
};

struct D3D9CmdGenerateMips
{
    IDirect3DBaseTexture9* texture;
};

struct D3D9CmdBufferWrite
{
    D3D9Buffer* dstBuffer;
    UINT        dstOffset;
    UINT        dataSize;
//  char        data[dataSize];
};

//TODO...

struct D3D9CmdSetShaderConstant
{
    UINT startRegister;
    UINT vector4Count;
//  VEC4 data[vector4Count];
};

struct D3D9CmdDraw
{
    D3DPRIMITIVETYPE    primitiveType;
    UINT                startVertex;
    UINT                primitiveCount;
};

struct D3D9CmdDrawIndexed
{
    D3DPRIMITIVETYPE    primitiveType;
    INT                 baseVertexIndex;
    UINT                minVertexIndex;
    UINT                numVertices;
    UINT                startIndex;
    UINT                primitiveCount;
};

struct D3D9CmdPushDebugGroup
{
    std::size_t length;
//  char        name[length];
};

//struct D3D9CmdPopDebugGroup {};


} // /namespace LLGL


#endif



// ================================================================================
