/*
 * D3D11Command.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_COMMAND_H
#define LLGL_D3D11_COMMAND_H


#include "../Direct3D11.h"
#include <cstdint>


namespace LLGL
{


class Resource;
class ResourceHeap;
class D3D11Buffer;
class D3D11BufferArray;
class D3D11ResourceHeap;
class D3D11PipelineState;

struct D3D11CmdSetVertexBuffer
{
    D3D11Buffer* buffer;
};

struct D3D11CmdSetVertexBufferArray
{
    D3D11BufferArray* bufferArray;
};

struct D3D11CmdSetIndexBuffer
{
    D3D11Buffer*    buffer;
    DXGI_FORMAT     format;
    UINT            offset;
};

struct D3D11CmdSetResourceHeap
{
    D3D11ResourceHeap*  resourceHeap;
    std::uint32_t       descriptorSet;
};

struct D3D11CmdSetResource
{
    std::uint32_t   descriptor;
    Resource*       resource;
};

struct D3D11CmdSetPipelineState
{
    D3D11PipelineState* pipelineState;
};

struct D3D11CmdSetBlendFactor
{
    FLOAT color[4];
};

struct D3D11CmdSetStencilRef
{
    UINT stencilRef;
};

struct D3D11CmdSetUniforms
{
    std::uint32_t   first;
    std::uint16_t   dataSize;
//  char            data[dataSize];
};

struct D3D11CmdDraw
{
    UINT    vertexCount;
    UINT    startVertexLocation;
};

struct D3D11CmdDrawIndexed
{
    UINT    indexCount;
    UINT    startIndexLocation;
    INT     baseVertexLocation;
};

struct D3D11CmdDrawInstanced
{
    UINT    vertexCountPerInstance;
    UINT    instanceCount;
    UINT    startVertexLocation;
    UINT    startInstanceLocation;
};

struct D3D11CmdDrawIndexedInstanced
{
    UINT    indexCountPerInstance;
    UINT    instanceCount;
    UINT    startIndexLocation;
    INT     baseVertexLocation;
    UINT    startInstanceLocation;
};

struct D3D11CmdDrawInstancedIndirect
{
    ID3D11Buffer*   bufferForArgs;
    UINT            alignedByteOffsetForArgs;
    UINT            numCommands;
    UINT            stride;
};

//D3D11CmdDrawIndexedInstancedIndirect = D3D11CmdDrawInstancedIndirect

struct D3D11CmdDispatch
{
    UINT threadGroupCountX;
    UINT threadGroupCountY;
    UINT threadGroupCountZ;
};

struct D3D11CmdDispatchIndirect
{
    ID3D11Buffer*   bufferForArgs;
    UINT            alignedByteOffsetForArgs;
};


} // /namespace LLGL


#endif



// ================================================================================
