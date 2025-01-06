/*
 * NullBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "NullBuffer.h"
#include "../../ResourceUtils.h"
#include "../../../Core/CoreUtils.h"
#include <algorithm>
#include <string.h>


namespace LLGL
{


constexpr NullBuffer::WordType g_uninitializedBufferWord = 0xDEADBEEF;

NullBuffer::NullBuffer(const BufferDescriptor& desc, const void* initialData) :
    Buffer { desc.bindFlags },
    desc   { desc           }
{
    /* Allocate word-aligned buffer and initialize with hex code as debug information */
    const std::size_t wordAlignedSize = GetAlignedSize(static_cast<std::size_t>(desc.size), sizeof(WordType));
    data_.resize(wordAlignedSize, g_uninitializedBufferWord);

    /* Allocate mapped buffer if CPU access flags are specified */
    if (desc.cpuAccessFlags != 0)
        mappedData_.resize(wordAlignedSize, g_uninitializedBufferWord);

    /* Initialize buffer with initial data */
    if (initialData != nullptr)
        Write(0, initialData, static_cast<std::size_t>(desc.size));

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void NullBuffer::SetDebugName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}

bool NullBuffer::GetNativeHandle(void* /*nativeHandle*/, std::size_t /*nativeHandleSize*/)
{
    return false; // dummy
}

BufferDescriptor NullBuffer::GetDesc() const
{
    return desc;
}

static bool IsRangeInsideBuffer(const NullBuffer& buffer, std::uint64_t offset, std::uint64_t size)
{
    /* Check for out-of-bounds and ensure there's no integer overflow with offset+size */
    return (offset < buffer.desc.size && offset + size <= buffer.desc.size && offset + size > offset);
}

bool NullBuffer::Read(std::uint64_t offset, void* data, std::uint64_t size)
{
    if (IsRangeInsideBuffer(*this, offset, size))
    {
        ::memcpy(data, GetBytesAt(offset), static_cast<std::size_t>(size));
        return true;
    }
    return false;
}

bool NullBuffer::Write(std::uint64_t offset, const void* data, std::uint64_t size)
{
    if (IsRangeInsideBuffer(*this, offset, size))
    {
        ::memcpy(GetBytesAt(offset), data, static_cast<std::size_t>(size));
        return true;
    }
    return false;
}

bool NullBuffer::CopyFromBuffer(std::uint64_t dstOffset, const NullBuffer& srcBuffer, std::uint64_t srcOffset, std::uint64_t size)
{
    if (IsRangeInsideBuffer(*this, dstOffset, size) && IsRangeInsideBuffer(srcBuffer, srcOffset, size))
    {
        ::memcpy(GetBytesAt(dstOffset), srcBuffer.GetBytesAt(srcOffset), static_cast<std::size_t>(size));
        return true;
    }
    return false;
}

bool NullBuffer::CpuAccessRead(std::uint64_t offset, void* data, std::uint64_t size)
{
    if ((desc.cpuAccessFlags & CPUAccessFlags::Read) != 0)
        return Read(offset, data, size);
    else
        return false;
}

bool NullBuffer::CpuAccessWrite(std::uint64_t offset, const void* data, std::uint64_t size)
{
    if ((desc.cpuAccessFlags & CPUAccessFlags::Write) != 0)
        return Write(offset, data, size);
    else
        return false;
}

void* NullBuffer::Map(const CPUAccess access, std::uint64_t offset, std::uint64_t length)
{
    /* Cannot map while data is already mapped */
    if (mapLength_ > 0)
        return nullptr;

    /* Check for out-of-bounds and ensure there's no integer overflow with offset+length */
    if (!(offset < desc.size && offset + length <= desc.size && offset + length > offset))
        return nullptr;

    const bool isWriteAccess = HasWriteAccess(access);
    const bool isReadAccess = HasReadAccess(access);

    if ((isWriteAccess && (desc.cpuAccessFlags & CPUAccessFlags::Write) == 0) ||
        (isReadAccess  && (desc.cpuAccessFlags & CPUAccessFlags::Read ) == 0))
    {
        /* Wrong CPU access for this buffer */
        return nullptr;
    }

    if (access == CPUAccess::WriteDiscard)
    {
        /* Discard all buffer content by filling buffer with uninitialized word */
        std::fill(data_.begin(), data_.end(), g_uninitializedBufferWord);
    }

    /* Map internal buffer to output data */
    mapAccess_ = access;
    mapOffset_ = static_cast<std::size_t>(offset);
    mapLength_ = static_cast<std::size_t>(length);

    if (isReadAccess)
    {
        /* Map buffer for reading */
        ::memcpy(GetMappedBytes(), GetBytesAt(mapOffset_), mapLength_);
    }

    return GetMappedBytes();
}

void NullBuffer::Unmap()
{
    if (mapLength_ > 0)
    {
        if (HasWriteAccess(mapAccess_))
        {
            /* Map buffer for writing */
            ::memcpy(GetBytesAt(mapOffset_), GetMappedBytes(), mapLength_);
        }
        mapLength_ = 0;
    }
}


} // /namespace LLGL



// ================================================================================
