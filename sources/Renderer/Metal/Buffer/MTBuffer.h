/*
 * MTBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_BUFFER_H
#define LLGL_MT_BUFFER_H


#import <Metal/Metal.h>

#include <LLGL/Buffer.h>


namespace LLGL
{


class MTBuffer : public Buffer
{

    public:

        MTBuffer(id<MTLDevice> device, const BufferDescriptor& desc, const void* initialData);
        ~MTBuffer();
    
        void Write(NSUInteger dstOffset, const void* data, NSUInteger dataSize);

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
        bool            isManaged_          = false;

};


} // /namespace LLGL


#endif



// ================================================================================
