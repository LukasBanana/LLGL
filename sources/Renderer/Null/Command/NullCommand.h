/*
 * NullCommand.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_NULL_COMMAND_H
#define LLGL_NULL_COMMAND_H


#include <LLGL/IndirectArguments.h>
#include <cstddef>
#include <cstdint>


namespace LLGL
{


class NullBuffer;
class NullTexture;


struct NullCmdBufferWrite
{
    NullBuffer* buffer;
    std::size_t offset;
    std::size_t size;
//  std::int8_t data[dataSize];
};

struct NullCmdCopySubresource
{
    Resource*       srcResource;
    std::uint32_t   srcSubresource;
    std::uint64_t   srcX;
    std::uint32_t   srcY;
    std::uint32_t   srcZ;
    Resource*       dstResource;
    std::uint32_t   dstSubresource;
    std::uint64_t   dstX;
    std::uint32_t   dstY;
    std::uint32_t   dstZ;
    std::uint64_t   width;
    std::uint32_t   height;
    std::uint32_t   depth;
    std::uint32_t   rowStride;
    std::uint32_t   layerStride;
};

struct NullCmdGenerateMips
{
    NullTexture*    texture;
    std::uint32_t   baseArrayLayer;
    std::uint32_t   numArrayLayers;
    std::uint32_t   baseMipLevel;
    std::uint32_t   numMipLevels;
};

//TODO...

struct NullCmdDraw
{
    DrawIndirectArguments   args;
    std::size_t             numVertexBuffers;
//  const NullBuffer*       vertexBuffers[numVertexBuffers];
};

struct NullCmdDrawIndexed
{
    DrawIndexedIndirectArguments    args;
    const NullBuffer*               indexBuffer;
    Format                          indexBufferFormat;
    std::uint64_t                   indexBufferOffset;
    std::size_t                     numVertexBuffers;
//  const NullBuffer*               vertexBuffers[numVertexBuffers];
};

struct NullCmdPushDebugGroup
{
    std::size_t length;
//  char        name[length];
};

//struct NullCmdPopDebugGroup {};


} // /namespace LLGL


#endif



// ================================================================================
