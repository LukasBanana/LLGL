/*
 * MTBuffer.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_BUFFER_H
#define LLGL_MT_BUFFER_H


#import <Metal/Metal.h>

#include <LLGL/Platform/Platform.h>
#include <LLGL/Buffer.h>


namespace LLGL
{


class MTBuffer final : public Buffer
{

    public:

        BufferDescriptor GetDesc() const override;

    public:

        MTBuffer(id<MTLDevice> device, const BufferDescriptor& desc, const void* initialData);
        ~MTBuffer();

        void Write(NSUInteger offset, const void* data, NSUInteger dataSize);
        void Read(NSUInteger offset, void* data, NSUInteger dataSize);

        void* Map(CPUAccess access);
        void* Map(CPUAccess access, NSUInteger offset, NSUInteger length);
        void Unmap();

        // Returns the native MTLBuffer object.
        inline id<MTLBuffer> GetNative() const
        {
            return native_;
        }

        // Returns true if the index buffer format has 16 bit indices.
        inline bool IsIndexType16Bits() const
        {
            return indexType16Bits_;
        }

    private:

        id<MTLBuffer>   native_             = nil;
        bool            indexType16Bits_    = false;
        #ifndef LLGL_OS_IOS
        bool            isManaged_          = false;
        #endif
        NSRange         mappedWriteRange_   = { 0, 0 };

};


} // /namespace LLGL


#endif



// ================================================================================
