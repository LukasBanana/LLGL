/*
 * MTIntermediateBuffer.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_INTERMEDIATE_BUFFER_H
#define LLGL_MT_INTERMEDIATE_BUFFER_H


#import <Metal/Metal.h>


namespace LLGL
{


// Helper class to manage an internal <MTLBuffer> for tessellation factors.
class MTIntermediateBuffer
{

    public:

        MTIntermediateBuffer(
            id<MTLDevice>       device,
            MTLResourceOptions  options     = MTLResourceStorageModePrivate,
            NSUInteger          alignment   = 16,
            NSUInteger          initialSize = 0
        );
        ~MTIntermediateBuffer();

        // Allocates a new buffer with the specified size.
        void Resize(NSUInteger size);

        // Allocates a new buffer if the specified size is larger than the previous one. In this case, the new size is multiplied by 1.5x.
        void Grow(NSUInteger size);

        // Returns the native MTLBuffer object.
        inline id<MTLBuffer> GetNative() const
        {
            return native_;
        }

        // Returns the mutable byte contents of the native buffer.
        inline void* GetBytes()
        {
            return [native_ contents];
        }

        // Returns the immutable byte contents of the native buffer.
        inline const void* GetBytes() const
        {
            return [native_ contents];
        }

    private:

        id<MTLDevice>       device_     = nil;
        id<MTLBuffer>       native_     = nil;
        MTLResourceOptions  options_    = MTLResourceStorageModePrivate;
        NSUInteger          alignment_  = 16;

};


} // /namespace LLGL


#endif



// ================================================================================
