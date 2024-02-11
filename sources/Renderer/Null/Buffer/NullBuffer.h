/*
 * NullBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_NULL_BUFFER_H
#define LLGL_NULL_BUFFER_H


#include <LLGL/Buffer.h>
#include <vector>
#include <string>


namespace LLGL
{


class NullBuffer final : public Buffer
{

    public:

        void SetDebugName(const char* name) override;

        BufferDescriptor GetDesc() const override;

    public:

        NullBuffer(const BufferDescriptor& desc, const void* initialData);

        bool Read(std::uint64_t offset, void* data, std::uint64_t size);
        bool Write(std::uint64_t offset, const void* data, std::uint64_t size);

        bool CpuAccessRead(std::uint64_t offset, void* data, std::uint64_t size);
        bool CpuAccessWrite(std::uint64_t offset, const void* data, std::uint64_t size);

        bool CopyFromBuffer(std::uint64_t dstOffset, const NullBuffer& srcBuffer, std::uint64_t srcOffset, std::uint64_t size);

        void* Map(const CPUAccess access, std::uint64_t offset, std::uint64_t length);
        void Unmap();

    public:

        // Data type for the internal buffer data.
        using WordType = std::uint32_t;

        const BufferDescriptor desc;

    private:

        inline char* GetBytes()
        {
            return reinterpret_cast<char*>(data_.data());
        }

        inline const char* GetBytes() const
        {
            return reinterpret_cast<const char*>(data_.data());
        }

        inline char* GetBytesAt(std::uint64_t offset)
        {
            return (GetBytes() + static_cast<std::size_t>(offset));
        }

        inline const char* GetBytesAt(std::uint64_t offset) const
        {
            return (GetBytes() + static_cast<std::size_t>(offset));
        }

        inline char* GetMappedBytes()
        {
            return (reinterpret_cast<char*>(mappedData_.data()) + mapOffset_);
        }

        inline const char* GetMappedBytes() const
        {
            return (reinterpret_cast<const char*>(mappedData_.data()) + mapOffset_);
        }

    private:

        std::string             label_;
        std::vector<WordType>   data_;
        std::vector<WordType>   mappedData_;
        std::size_t             mapOffset_  = 0;
        std::size_t             mapLength_  = 0;
        CPUAccess               mapAccess_  = CPUAccess::ReadOnly;

};


} // /namespace LLGL


#endif



// ================================================================================
