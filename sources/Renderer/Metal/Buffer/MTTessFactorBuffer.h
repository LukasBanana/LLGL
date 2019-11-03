/*
 * MTTessFactorBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_TESS_FACTOR_BUFFER_H
#define LLGL_MT_TESS_FACTOR_BUFFER_H


#import <Metal/Metal.h>


namespace LLGL
{


// Helper class to manage an internal <MTLBuffer> for tessellation factors.
class MTTessFactorBuffer
{

    public:

        MTTessFactorBuffer(id<MTLDevice> device, NSUInteger initialSize = 0);
        ~MTTessFactorBuffer();

        // Allocates a new buffer if the specified size is larger than the previous one.
        void Grow(NSUInteger size);

        // Returns the native MTLBuffer object.
        inline id<MTLBuffer> GetNative() const
        {
            return native_;
        }

    private:

        id<MTLDevice> device_ = nil;
        id<MTLBuffer> native_ = nil;

};


} // /namespace LLGL


#endif



// ================================================================================
