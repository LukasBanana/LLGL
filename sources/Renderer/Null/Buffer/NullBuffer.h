/*
 * NullBuffer.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

        void SetName(const char* name) override;

        BufferDescriptor GetDesc() const override;

    public:

        NullBuffer(const BufferDescriptor& desc, const void* initialData);

        bool Read(std::uint64_t offset, void* data, std::uint64_t size);
        bool Write(std::uint64_t offset, const void* data, std::uint64_t size);

        bool CpuAccessRead(std::uint64_t offset, void* data, std::uint64_t size);
        bool CpuAccessWrite(std::uint64_t offset, const void* data, std::uint64_t size);

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

        inline char* GetBytesAt(std::uint64_t offset)
        {
            return (reinterpret_cast<char*>(data_.data()) + static_cast<std::size_t>(offset));
        }

        inline char* GetMappedBytes()
        {
            return (reinterpret_cast<char*>(mappedData_.data()) + mapOffset_);
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
