/*
 * BufferFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/BufferFlags.h>


namespace LLGL
{


LLGL_EXPORT bool IsRWBuffer(const StorageBufferType type)
{
    return (type >= StorageBufferType::RWBuffer);
}

LLGL_EXPORT bool IsTypedBuffer(const StorageBufferType type)
{
    return
    (
        type == StorageBufferType::Buffer   ||
        type == StorageBufferType::RWBuffer
    );
}

LLGL_EXPORT bool IsStructuredBuffer(const StorageBufferType type)
{
    return
    (
        type == StorageBufferType::StructuredBuffer         ||
        type == StorageBufferType::RWStructuredBuffer       ||
        type >= StorageBufferType::AppendStructuredBuffer
    );
}

LLGL_EXPORT bool IsByteAddressBuffer(const StorageBufferType type)
{
    return
    (
        type == StorageBufferType::ByteAddressBuffer    ||
        type == StorageBufferType::RWByteAddressBuffer
    );
}


} // /namespace LLGL



// ================================================================================
