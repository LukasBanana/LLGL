/*
 * DbgBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DBG_BUFFER_H
#define LLGL_DBG_BUFFER_H


#include <LLGL/Buffer.h>
#include <LLGL/Container/SmallVector.h>
#include <string>


namespace LLGL
{


class DbgBuffer final : public Buffer
{

        const SmallVector<VertexAttribute> vertexAttribs_;

    public:

        void SetDebugName(const char* name) override;

        BufferDescriptor GetDesc() const override;

    public:

        DbgBuffer(Buffer& instance, const BufferDescriptor& desc);

        void OnMap(const CPUAccess access, std::uint64_t offset, std::uint64_t length);
        void OnUnmap();

        // Returns true if this buffer is currently mapped into CPU memory space.
        bool IsMappedForCPUAccess() const;

    public:

        Buffer&                 instance;
        const BufferDescriptor  desc;
        std::string             label;
        std::uint64_t           elements    = 0;
        bool                    initialized = false;

    private:

        CPUAccess               mappedAccess_   = CPUAccess::ReadOnly;
        std::uint64_t           mappedRange_[2] = { 0, 0 };

};


} // /namespace LLGL


#endif



// ================================================================================
