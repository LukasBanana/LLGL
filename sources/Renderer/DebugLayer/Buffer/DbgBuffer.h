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

        void SetName(const char* name) override;

        BufferDescriptor GetDesc() const override;

    public:

        DbgBuffer(Buffer& instance, const BufferDescriptor& desc);

    public:

        Buffer&                 instance;
        const BufferDescriptor  desc;
        std::string             label;
        std::uint64_t           elements    = 0;
        bool                    initialized = false;
        bool                    mapped      = false;

};


} // /namespace LLGL


#endif



// ================================================================================
