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

/*struct D3D9CmdBufferWrite
{
    D3D9VertexBuffer*   vertexBuffer;
    D3D9IndexBuffer*    indexBuffer;
    std::size_t         offset;
    std::size_t         size;
//  std::int8_t         data[dataSize];
};*/

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

struct D3D9CmdBindProgrammablePSO
{
    IDirect3DVertexDeclaration9*    vertexDeclaration;
    IDirect3DVertexShader9*         vertexShader;
    IDirect3DPixelShader9*          pixelShader;
};

struct D3D9CmdSetRenderStates
{
    UINT                    numRenderStates;
    struct RenderState
    {
        D3DRENDERSTATETYPE  type;
        DWORD               value;
    };
//  RenderState             renderStates[numRenderStates];
};

//TODO...

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
