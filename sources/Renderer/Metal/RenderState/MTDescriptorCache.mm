/*
 * MTDescriptorCache.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTDescriptorCache.h"
#include "MTPipelineLayout.h"
#include "../Buffer/MTBuffer.h"
#include "../Texture/MTTexture.h"
#include "../Texture/MTSampler.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>
#include <string.h>


namespace LLGL
{


MTDescriptorCache::MTDescriptorCache()
{
    Clear();
}

void MTDescriptorCache::Reset(const MTPipelineLayout* pipelineLayout)
{
    if (pipelineLayout != nullptr)
    {
        /* Store reference to dynamic binding layouts */
        layouts_ = pipelineLayout->GetDynamicBindings();
        LLGL_ASSERT(layouts_.size() <= 0xFF);
        bindings_.resize(layouts_.size());
    }
    else
    {
        /* Clear layout reference */
        layouts_ = {};
    }
    Reset();
}

void MTDescriptorCache::Reset()
{
    /* Invalidate all resources */
    dirtyBindings_[0]   = ~0ull;
    dirtyBindings_[1]   = ~0ull;
    dirtyBindings_[2]   = ~0ull;
    dirtyBindings_[3]   = ~0ull;
    dirtyRange_[0]      = 0;
    dirtyRange_[1]      = static_cast<std::uint8_t>(bindings_.size());
}

// private
void MTDescriptorCache::Clear()
{
    /* Clear dirty range and resource bits */
    dirtyBindings_[0]   = 0;
    dirtyBindings_[1]   = 0;
    dirtyBindings_[2]   = 0;
    dirtyBindings_[3]   = 0;
    dirtyRange_[0]      = 0xFF;
    dirtyRange_[1]      = 0;
}

void MTDescriptorCache::SetResource(std::uint32_t descriptor, Resource& resource)
{
    if (descriptor >= layouts_.size())
        return /*Out of range*/;

    const MTDynamicResourceLayout& layout = layouts_[descriptor];
    if (layout.type != resource.GetResourceType())
        return /*Type mismatch*/;

    LLGL_ASSERT(bindings_.size() >= layouts_.size());

    switch (layout.type)
    {
        case ResourceType::Undefined:
        break;

        case ResourceType::Buffer:
        {
            auto& bufferMT = LLGL_CAST(MTBuffer&, resource);
            bindings_[descriptor] = bufferMT.GetNative();
        }
        break;

        case ResourceType::Texture:
        {
            auto& textureMT = LLGL_CAST(MTTexture&, resource);
            bindings_[descriptor] = textureMT.GetNative();
        }
        break;

        case ResourceType::Sampler:
        {
            auto& samplerMT = LLGL_CAST(MTSampler&, resource);
            bindings_[descriptor] = samplerMT.GetNative();
        }
        break;
    }

    InvalidateBinding(static_cast<std::uint8_t>(descriptor));
}

void MTDescriptorCache::FlushGraphicsResources(id<MTLRenderCommandEncoder> renderEncoder)
{
    if (dirtyRange_[0] < dirtyRange_[1])
    {
        LLGL_ASSERT(bindings_.size() >= dirtyRange_[1]);
        LLGL_ASSERT(layouts_.size() >= dirtyRange_[1]);
        for_subrange(i, dirtyRange_[0], dirtyRange_[1])
        {
            if (IsBindingInvalidated(i))
                BindGraphicsResource(renderEncoder, layouts_[i], bindings_[i]);
        }
        Clear();
    }
}

void MTDescriptorCache::FlushGraphicsResourcesForced(id<MTLRenderCommandEncoder> renderEncoder)
{
    LLGL_ASSERT(bindings_.size() >= layouts_.size());
    for_range(i, layouts_.size())
        BindGraphicsResource(renderEncoder, layouts_[i], bindings_[i]);
    Clear();
}

void MTDescriptorCache::FlushComputeResources(id<MTLComputeCommandEncoder> computeEncoder)
{
    if (dirtyRange_[0] < dirtyRange_[1])
    {
        LLGL_ASSERT(bindings_.size() >= dirtyRange_[1]);
        LLGL_ASSERT(layouts_.size() >= dirtyRange_[1]);
        for_subrange(i, dirtyRange_[0], dirtyRange_[1])
        {
            if (IsBindingInvalidated(i))
                BindComputeResource(computeEncoder, layouts_[i], bindings_[i]);
        }
        Clear();
    }
}

void MTDescriptorCache::FlushComputeResourcesForced(id<MTLComputeCommandEncoder> computeEncoder)
{
    LLGL_ASSERT(bindings_.size() >= layouts_.size());
    for_range(i, layouts_.size())
        BindComputeResource(computeEncoder, layouts_[i], bindings_[i]);
    Clear();
}


/*
 * ======= Private: =======
 */

void MTDescriptorCache::BindGraphicsResource(id<MTLRenderCommandEncoder> renderEncoder, const MTDynamicResourceLayout& layout, id resource)
{
    switch (layout.type)
    {
        case ResourceType::Undefined:
        break;

        case ResourceType::Buffer:
        {
            if ((layout.stages & StageFlags::VertexStage) != 0)
            {
                [renderEncoder
                    setVertexBuffer:    static_cast<id<MTLBuffer>>(resource)
                    offset:             0
                    atIndex:            layout.slot
                ];
            }
            if ((layout.stages & StageFlags::FragmentStage) != 0)
            {
                [renderEncoder
                    setFragmentBuffer:  static_cast<id<MTLBuffer>>(resource)
                    offset:             0
                    atIndex:            layout.slot
                ];
            }
        }
        break;

        case ResourceType::Texture:
        {
            if ((layout.stages & StageFlags::VertexStage) != 0)
            {
                [renderEncoder
                    setVertexTexture:   static_cast<id<MTLTexture>>(resource)
                    atIndex:            layout.slot
                ];
            }
            if ((layout.stages & StageFlags::FragmentStage) != 0)
            {
                [renderEncoder
                    setFragmentTexture: static_cast<id<MTLTexture>>(resource)
                    atIndex:            layout.slot
                ];
            }
        }
        break;

        case ResourceType::Sampler:
        {
            if ((layout.stages & StageFlags::VertexStage) != 0)
            {
                [renderEncoder
                    setVertexSamplerState:  static_cast<id<MTLSamplerState>>(resource)
                    atIndex:                layout.slot
                ];
            }
            if ((layout.stages & StageFlags::FragmentStage) != 0)
            {
                [renderEncoder
                    setFragmentSamplerState:    static_cast<id<MTLSamplerState>>(resource)
                    atIndex:                    layout.slot
                ];
            }
        }
        break;
    }
}

void MTDescriptorCache::BindComputeResource(id<MTLComputeCommandEncoder> computeEncoder, const MTDynamicResourceLayout& layout, id resource)
{
    switch (layout.type)
    {
        case ResourceType::Undefined:
        break;

        case ResourceType::Buffer:
        {
            if ((layout.stages & StageFlags::ComputeStage) != 0)
            {
                [computeEncoder
                    setBuffer:  static_cast<id<MTLBuffer>>(resource)
                    offset:     0
                    atIndex:    layout.slot
                ];
            }
        }
        break;

        case ResourceType::Texture:
        {
            if ((layout.stages & StageFlags::ComputeStage) != 0)
            {
                [computeEncoder
                    setTexture: static_cast<id<MTLTexture>>(resource)
                    atIndex:    layout.slot
                ];
            }
        }
        break;

        case ResourceType::Sampler:
        {
            if ((layout.stages & StageFlags::ComputeStage) != 0)
            {
                [computeEncoder
                    setSamplerState:    static_cast<id<MTLSamplerState>>(resource)
                    atIndex:            layout.slot
                ];
            }
        }
        break;
    }
}

void MTDescriptorCache::InvalidateBinding(std::uint8_t index)
{
    dirtyBindings_[index / 64] |= (1u << (index % 64));
    dirtyRange_[0] = std::min<std::uint8_t>(dirtyRange_[0], index);
    dirtyRange_[1] = std::max<std::uint8_t>(dirtyRange_[1], index + 1u);
}


} // /namespace LLGL



// ================================================================================
