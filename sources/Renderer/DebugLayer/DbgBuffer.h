/*
 * DbgBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_BUFFER_H
#define LLGL_DBG_BUFFER_H


#include <LLGL/Buffer.h>
#include <string>


namespace LLGL
{


class DbgBuffer final : public Buffer
{

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
