/*
 * MTDescriptorCache.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_DESCRIPTOR_CACHE_H
#define LLGL_MT_DESCRIPTOR_CACHE_H


#import <Metal/Metal.h>

#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <vector>


namespace LLGL
{


class MTPipelineLayout;

struct MTDynamicResourceLayout
{
    ResourceType    type    = ResourceType::Undefined;
    NSUInteger      slot    = 0;
    long            stages  = 0;
};

// Stores a fixed number of Metal resource descriptors. Used by MTPipelineState.
class MTDescriptorCache
{

    public:

        // Initialises the cache by clearing all dirty bits.
        MTDescriptorCache();

        // Resets the binding layouts and dirty bits.
        void Reset(const MTPipelineLayout* pipelineLayout);

        // Resets the dirty bits which will bind all resources on the next flush, i.e. IsInvalidated() returns true.
        void Reset();

        // Sets the specified resource in this cache.
        void SetResource(std::uint32_t descriptor, Resource& resource);

        // Flushes the pending descriptors to the specified command encoder.
        void FlushGraphicsResources(id<MTLRenderCommandEncoder> renderEncoder);
        void FlushGraphicsResourcesForced(id<MTLRenderCommandEncoder> renderEncoder);
        void FlushComputeResources(id<MTLComputeCommandEncoder> computeEncoder);
        void FlushComputeResourcesForced(id<MTLComputeCommandEncoder> computeEncoder);

        // Returns true if this cache has been invalidated.
        inline bool IsInvalidated() const
        {
            return (dirtyRange_[0] < dirtyRange_[1]);
        }

        // Returns true if this cache is empty and does not contain any binding layouts.
        inline bool IsEmpty() const
        {
            return layouts_.empty();
        }

    private:

        void BuildResourceBindings(const ArrayView<MTDynamicResourceLayout>& bindings);

        void BindGraphicsResource(id<MTLRenderCommandEncoder> renderEncoder, const MTDynamicResourceLayout& layout, id resource);
        void BindComputeResource(id<MTLComputeCommandEncoder> computeEncoder, const MTDynamicResourceLayout& layout, id resource);

        // Marks the specified binding as invalidated.
        void InvalidateBinding(std::uint8_t index);

        // Clears all dirty bits, i.e IsInvalidated() returns false.
        void Clear();

        // Returns true if the specified binding is invalidated.
        inline bool IsBindingInvalidated(std::uint8_t index) const
        {
            return (((dirtyBindings_[index / 64] >> (index % 64)) & 0x1) != 0);
        }

    private:

        ArrayView<MTDynamicResourceLayout>  layouts_;
        std::vector<id>                     bindings_;
        std::uint64_t                       dirtyBindings_[4]   = {};
        std::uint8_t                        dirtyRange_[2]      = {};

};


} // /namespace LLGL


#endif



// ================================================================================
