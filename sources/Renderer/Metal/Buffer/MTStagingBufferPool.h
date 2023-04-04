/*
 * MTStagingBufferPool.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_STAGING_BUFFER_POOL_H
#define LLGL_MT_STAGING_BUFFER_POOL_H


#import <Metal/Metal.h>

#include "MTStagingBuffer.h"
#include <vector>


namespace LLGL
{


class MTStagingBufferPool
{

    public:

        MTStagingBufferPool(id<MTLDevice> device, NSUInteger chunkSize);
    
        void Reset();
    
        void Write(
            const void*     data,
            NSUInteger      dataSize,
            id<MTLBuffer>&  srcBuffer,
            NSUInteger&     srcOffset
        );

    private:
    
        void AllocChunk(NSUInteger minChunkSize);
    
    private:

        id<MTLDevice>                   device_     = nil;
        std::vector<MTStagingBuffer>    chunks_;
        std::size_t                     chunkIdx_   = 0;
        NSUInteger                      chunkSize_  = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
