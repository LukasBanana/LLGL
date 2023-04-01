/*
 * MTDescriptorCache.mm
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTDescriptorCache.h"
#include "../Buffer/MTBuffer.h"
#include "../Texture/MTTexture.h"
#include "../Texture/MTSampler.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Misc/ForRange.h>
#include <algorithm>
#include <string.h>


namespace LLGL
{


MTDescriptorCache::MTDescriptorCache(const ArrayView<MTDynamicResourceLayout>& bindings)
{
    LLGL_ASSERT(bindings.size() <= 0xFF);
    BuildResourceBindings(bindings);
    Clear();
}

void MTDescriptorCache::Reset()
{
    /* Invalidate all resources */
    dirtyRange_[0] = 0;
    dirtyRange_[1] = static_cast<std::uint8_t>(bindings_.size());
    dirtyBindings_[0] = ~0ull;
    dirtyBindings_[1] = ~0ull;
    dirtyBindings_[2] = ~0ull;
    dirtyBindings_[3] = ~0ull;
}

void MTDescriptorCache::Clear()
{
    /* Clear dirty range and resource bits */
    dirtyRange_[0] = ~0u;
    dirtyRange_[1] = 0;
    dirtyBindings_[0] = 0;
    dirtyBindings_[1] = 0;
    dirtyBindings_[2] = 0;
    dirtyBindings_[3] = 0;
}

void MTDescriptorCache::SetResource(std::uint32_t descriptor, Resource& resource)
{
    if (descriptor >= bindings_.size())
        return /*Out of range*/;

    auto& binding = bindings_[descriptor];
    if (binding.layout.type != resource.GetResourceType())
        return /*Type mismatch*/;

    switch (binding.layout.type)
    {
        case ResourceType::Undefined:
        break;

        case ResourceType::Buffer:
        {
            auto& bufferMT = LLGL_CAST(MTBuffer&, resource);
            binding.resource = bufferMT.GetNative();
        }
        break;

        case ResourceType::Texture:
        {
            auto& textureMT = LLGL_CAST(MTTexture&, resource);
            binding.resource = textureMT.GetNative();
        }
        break;

        case ResourceType::Sampler:
        {
            auto& samplerMT = LLGL_CAST(MTSampler&, resource);
            binding.resource = samplerMT.GetNative();
        }
        break;
    }

    InvalidateBinding(static_cast<std::uint8_t>(descriptor));
}

void MTDescriptorCache::FlushGraphicsResources(id<MTLRenderCommandEncoder> renderEncoder)
{
    if (dirtyRange_[0] < dirtyRange_[1])
    {
        for_subrange(i, dirtyRange_[0], dirtyRange_[1])
        {
            if (IsBindingInvalidated(i))
                BindGraphicsResource(renderEncoder, bindings_[i]);
        }
        Clear();
    }
}

void MTDescriptorCache::FlushGraphicsResourcesForced(id<MTLRenderCommandEncoder> renderEncoder)
{
    for (const auto& binding : bindings_)
        BindGraphicsResource(renderEncoder, binding);
    Clear();
}

void MTDescriptorCache::FlushComputeResources(id<MTLComputeCommandEncoder> computeEncoder)
{
    if (dirtyRange_[0] < dirtyRange_[1])
    {
        for_subrange(i, dirtyRange_[0], dirtyRange_[1])
        {
            if (IsBindingInvalidated(i))
                BindComputeResource(computeEncoder, bindings_[i]);
        }
        Clear();
    }
}

void MTDescriptorCache::FlushComputeResourcesForced(id<MTLComputeCommandEncoder> computeEncoder)
{
    for (const auto& binding : bindings_)
        BindComputeResource(computeEncoder, binding);
    Clear();
}


/*
 * ======= Private: =======
 */

void MTDescriptorCache::BuildResourceBindings(const ArrayView<MTDynamicResourceLayout>& bindings)
{
    bindings_.resize(bindings.size());
    for_range(i, bindings.size())
        bindings_[i].layout = bindings[i];
}

void MTDescriptorCache::BindGraphicsResource(id<MTLRenderCommandEncoder> renderEncoder, const MTDynamicResourceBinding& binding)
{
    switch (binding.layout.type)
    {
        case ResourceType::Undefined:
        break;

        case ResourceType::Buffer:
        {
            if ((binding.layout.stages & StageFlags::VertexStage) != 0)
            {
                [renderEncoder
                    setVertexBuffer:    static_cast<id<MTLBuffer>>(binding.resource)
                    offset:             0
                    atIndex:            binding.layout.slot
                ];
            }
            if ((binding.layout.stages & StageFlags::FragmentStage) != 0)
            {
                [renderEncoder
                    setFragmentBuffer:  static_cast<id<MTLBuffer>>(binding.resource)
                    offset:             0
                    atIndex:            binding.layout.slot
                ];
            }
        }
        break;

        case ResourceType::Texture:
        {
            if ((binding.layout.stages & StageFlags::VertexStage) != 0)
            {
                [renderEncoder
                    setVertexTexture:   static_cast<id<MTLTexture>>(binding.resource)
                    atIndex:            binding.layout.slot
                ];
            }
            if ((binding.layout.stages & StageFlags::FragmentStage) != 0)
            {
                [renderEncoder
                    setFragmentTexture: static_cast<id<MTLTexture>>(binding.resource)
                    atIndex:            binding.layout.slot
                ];
            }
        }
        break;

        case ResourceType::Sampler:
        {
            if ((binding.layout.stages & StageFlags::VertexStage) != 0)
            {
                [renderEncoder
                    setVertexSamplerState:  static_cast<id<MTLSamplerState>>(binding.resource)
                    atIndex:                binding.layout.slot
                ];
            }
            if ((binding.layout.stages & StageFlags::FragmentStage) != 0)
            {
                [renderEncoder
                    setFragmentSamplerState:    static_cast<id<MTLSamplerState>>(binding.resource)
                    atIndex:                    binding.layout.slot
                ];
            }
        }
        break;
    }
}

void MTDescriptorCache::BindComputeResource(id<MTLComputeCommandEncoder> computeEncoder, const MTDynamicResourceBinding& binding)
{
    switch (binding.layout.type)
    {
        case ResourceType::Undefined:
        break;

        case ResourceType::Buffer:
        {
            if ((binding.layout.stages & StageFlags::ComputeStage) != 0)
            {
                [computeEncoder
                    setBuffer:  static_cast<id<MTLBuffer>>(binding.resource)
                    offset:     0
                    atIndex:    binding.layout.slot
                ];
            }
        }
        break;

        case ResourceType::Texture:
        {
            if ((binding.layout.stages & StageFlags::ComputeStage) != 0)
            {
                [computeEncoder
                    setTexture: static_cast<id<MTLTexture>>(binding.resource)
                    atIndex:    binding.layout.slot
                ];
            }
        }
        break;

        case ResourceType::Sampler:
        {
            if ((binding.layout.stages & StageFlags::ComputeStage) != 0)
            {
                [computeEncoder
                    setSamplerState:    static_cast<id<MTLSamplerState>>(binding.resource)
                    atIndex:            binding.layout.slot
                ];
            }
        }
        break;
    }
}

void MTDescriptorCache::InvalidateBinding(std::uint8_t index)
{
    dirtyBindings_[index / 64] |= (1u << (index % 64));
    dirtyRange_[0] = std::min(dirtyRange_[0], index);
    dirtyRange_[1] = std::max(dirtyRange_[1], index);
}


} // /namespace LLGL



// ================================================================================
