/*
 * MTConstantsCache.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_CONSTANTS_CACHE_H
#define LLGL_MT_CONSTANTS_CACHE_H


#import <Metal/Metal.h>

#include "MTConstantsCacheLayout.h"
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/DynamicArray.h>


namespace LLGL
{


// Manages the shader constants data for uniforms. Maximum size of such a cache is 4KB (as per Metal spec.).
class MTConstantsCache
{

    public:

        MTConstantsCache() = default;

        // Resets the cache layout and dirty bits.
        void Reset(const MTConstantsCacheLayout* layout);

        // Resets the dirty bits which will bind all resources on the next flush, i.e. IsInvalidated() returns true.
        void Reset();

        // Sets the specified resource in this cache.
        void SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize);

        // Flushes the pending descriptors to the specified command encoder.
        void FlushGraphicsResources(id<MTLRenderCommandEncoder> renderEncoder);
        void FlushGraphicsResourcesForced(id<MTLRenderCommandEncoder> renderEncoder);
        void FlushComputeResources(id<MTLComputeCommandEncoder> computeEncoder);
        void FlushComputeResourcesForced(id<MTLComputeCommandEncoder> computeEncoder);

        // Returns true if this cache has been invalidated.
        inline bool IsInvalidated() const
        {
            return (dirtyBits_.bits != 0);
        }

        // Returns whether this cache is empty.
        inline bool IsEmpty() const
        {
            return constantsMap_.empty();
        }

    private:

        using ConstantLocation  = MTConstantsCacheLayout::ConstantLocation;
        using ConstantBuffer    = MTConstantsCacheLayout::ConstantBuffer;

    private:

        ArrayView<ConstantLocation> constantsMap_;
        ArrayView<ConstantBuffer>   constantBuffers_;

        DynamicByteArray            constants_;

        union
        {
            std::uint8_t bits;
            struct
            {
                std::uint8_t graphics : 1;
                std::uint8_t compute  : 1;
            };
        }
        dirtyBits_;

};


} // /namespace LLGL


#endif



// ================================================================================
