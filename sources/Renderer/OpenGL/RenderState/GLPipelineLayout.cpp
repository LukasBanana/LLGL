/*
 * GLPipelineLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLPipelineLayout.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../../ResourceUtils.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


template <typename TContainer>
bool HasAnyNamedEntries(const TContainer& container)
{
    for (const auto& entry : container)
    {
        if (!entry.name.empty())
            return true;
    }
    return false;
}

// Converts the specified barrier flags into GLbitfield for glMemoryBarrier
static GLbitfield ToMemoryBarrierBitfield(long barrierFlags)
{
    GLbitfield barriers = 0;

    #if LLGL_GLEXT_MEMORY_BARRIERS

    if (HasExtension(GLExt::ARB_shader_image_load_store))
    {
        if ((barrierFlags & BarrierFlags::StorageBuffer) != 0)
            barriers |= GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT;
        if ((barrierFlags & BarrierFlags::StorageTexture) != 0)
            barriers |= GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
    }

    #endif // /LLGL_GLEXT_MEMORY_BARRIERS

    return barriers;
}

// Returns true if the specified pipeline layout descriptor contains any names for heap and dynamic resources.
static bool HasAnyNamedResourceBindings(const PipelineLayoutDescriptor& desc)
{
    return
    (
        HasAnyNamedEntries(desc.heapBindings)   ||
        HasAnyNamedEntries(desc.bindings)       ||
        HasAnyNamedEntries(desc.staticSamplers)
    );
}

GLPipelineLayout::GLPipelineLayout(const PipelineLayoutDescriptor& desc) :
    uniforms_         { desc.uniforms                              },
    barriers_         { ToMemoryBarrierBitfield(desc.barrierFlags) },
    hasNamedBindings_ { HasAnyNamedResourceBindings(desc)          }
{
    resourceNames_.reserve(desc.bindings.size() + desc.staticSamplers.size());

    /* First build combined texture-samplers, so that the first N entries in the 'combinedSamplerSlots_' are reserved for the input descriptors */
    BuildCombinedSamplerNames(desc);

    /* Then build resource bindings as they have an explicit offset into the 'combinedSamplerSlots_' slot (if they refer to them) */
    BuildHeapResourceBindings(desc);
    BuildDynamicResourceBindings(desc);
    BuildStaticSamplers(desc);
}

std::uint32_t GLPipelineLayout::GetNumHeapBindings() const
{
    return static_cast<std::uint32_t>(heapBindings_.size());
}

std::uint32_t GLPipelineLayout::GetNumBindings() const
{
    return static_cast<std::uint32_t>(bindings_.size());
}

std::uint32_t GLPipelineLayout::GetNumStaticSamplers() const
{
    return static_cast<std::uint32_t>(std::max(staticSamplers_.size(), staticEmulatedSamplers_.size()));
}

std::uint32_t GLPipelineLayout::GetNumUniforms() const
{
    return static_cast<std::uint32_t>(uniforms_.size());
}

void GLPipelineLayout::BindStaticSamplers(GLStateManager& stateMngr) const
{
    if (!staticSamplerSlots_.empty())
    {
        if (!HasNativeSamplers())
        {
            for_range(i, staticSamplerSlots_.size())
                stateMngr.BindEmulatedSampler(staticSamplerSlots_[i], *staticEmulatedSamplers_[i]);
        }
        else
        {
            for_range(i, staticSamplerSlots_.size())
                stateMngr.BindSampler(staticSamplerSlots_[i], staticSamplers_[i]->GetID());
        }
    }
}


/*
 * ======= Private: =======
 */

static GLResourceType ToGLResourceType(const BindingDescriptor& desc)
{
    switch (desc.type)
    {
        case ResourceType::Buffer:
            if ((desc.bindFlags & BindFlags::ConstantBuffer) != 0)
                return GLResourceType_UBO;
            if ((desc.bindFlags & (BindFlags::Sampled | BindFlags::Storage)) != 0)
                return GLResourceType_Buffer;
            break;

        case ResourceType::Texture:
            if ((desc.bindFlags & BindFlags::Sampled) != 0)
                return GLResourceType_Texture;
            if ((desc.bindFlags & BindFlags::Storage) != 0)
                return GLResourceType_Image;
            break;

        case ResourceType::Sampler:
            if (!HasNativeSamplers())
                return GLResourceType_EmulatedSampler;
            else
                return GLResourceType_Sampler;
            break;

        default:
            break;
    }
    return GLResourceType_Invalid;
}

void GLPipelineLayout::BuildHeapResourceBindings(const PipelineLayoutDescriptor& pipelineLayoutDesc)
{
    auto expandedHeapBindings = GetExpandedHeapDescriptors(pipelineLayoutDesc.heapBindings);
    heapBindings_.reserve(expandedHeapBindings.size());

    for (const BindingDescriptor& desc : expandedHeapBindings)
    {
        GLHeapResourceBinding newBinding;
        {
            newBinding.name         = desc.name.c_str();
            newBinding.type         = desc.type;
            newBinding.bindFlags    = desc.bindFlags;
            newBinding.stageFlags   = desc.stageFlags;
            newBinding.arraySize    = desc.arraySize;

            /* Try to build slot for combined texture-sampler first */
            std::uint32_t combiners = 0;
            if (!BuildCombinedSamplerSlots(pipelineLayoutDesc, desc.type, desc.name, newBinding.slot, combiners))
            {
                /* Otherwise, use explicit binding slot */
                newBinding.slot = static_cast<GLuint>(desc.slot.index);
            }
            newBinding.combiners = combiners;
        }
        heapBindings_.push_back(std::move(newBinding));
    }
}

void GLPipelineLayout::BuildDynamicResourceBindings(const PipelineLayoutDescriptor& pipelineLayoutDesc)
{
    bindings_.reserve(pipelineLayoutDesc.bindings.size());
    std::uint32_t ssboCounter = 0;

    for (const BindingDescriptor& desc : pipelineLayoutDesc.bindings)
    {
        GLPipelineResourceBinding newBinding;
        {
            newBinding.type = ToGLResourceType(desc);

            /* Try to build slot for combined texture-sampler first */
            std::uint32_t combiners = 0;
            if (!BuildCombinedSamplerSlots(pipelineLayoutDesc, desc.type, desc.name, newBinding.slot, combiners))
            {
                /* Otherwise, use explicit binding slot */
                newBinding.slot = static_cast<GLuint>(desc.slot.index);
            }
            newBinding.combiners = combiners;

            /* Store index to storage buffer  */
            if (newBinding.IsSSBO())
                newBinding.ssboIndex = ssboCounter++;
            else
                newBinding.ssboIndex = 0xFFFF;
        }
        bindings_.push_back(newBinding);
        resourceNames_.push_back(desc.name.c_str());
    }
}

void GLPipelineLayout::BuildStaticSamplers(const PipelineLayoutDescriptor& pipelineLayoutDesc)
{
    const std::vector<StaticSamplerDescriptor>& staticSamplerDescs = pipelineLayoutDesc.staticSamplers;
    staticSamplerSlots_.reserve(staticSamplerDescs.size());
    if (!HasNativeSamplers())
    {
        staticEmulatedSamplers_.reserve(staticSamplerDescs.size());
        for (const StaticSamplerDescriptor& desc : staticSamplerDescs)
        {
            /* Create emulated sampler and store slot and name separately */
            GLEmulatedSamplerSPtr sampler = std::make_shared<GLEmulatedSampler>();
            sampler->SamplerParameters(desc.sampler);

            const std::uint32_t numCombinedSlots = BuildCombinedStaticSamplerSlots(pipelineLayoutDesc, desc.name);
            if (numCombinedSlots > 0)
            {
                for_range(i, numCombinedSlots)
                    staticEmulatedSamplers_.push_back(sampler);
            }
            else
            {
                staticEmulatedSamplers_.push_back(std::move(sampler));
                staticSamplerSlots_.push_back(desc.slot.index);
            }
        }
    }
    else
    {
        staticSamplers_.reserve(staticSamplerDescs.size());
        for (const StaticSamplerDescriptor& desc : staticSamplerDescs)
        {
            /* Create native sampler (GL 3.3+) and store slot and name separately */
            GLSamplerSPtr sampler = std::make_shared<GLSampler>();
            sampler->SamplerParameters(desc.sampler);

            const std::uint32_t numCombinedSlots = BuildCombinedStaticSamplerSlots(pipelineLayoutDesc, desc.name);
            if (numCombinedSlots > 0)
            {
                for_range(i, numCombinedSlots)
                    staticSamplers_.push_back(sampler);
            }
            else
            {
                staticSamplers_.push_back(std::move(sampler));
                staticSamplerSlots_.push_back(desc.slot.index);
            }
        }
    }
}

void GLPipelineLayout::BuildCombinedSamplerNames(const PipelineLayoutDescriptor& pipelineLayoutDesc)
{
    resourceNames_.reserve(pipelineLayoutDesc.combinedTextureSamplers.size());
    combinedSamplerSlots_.reserve(pipelineLayoutDesc.combinedTextureSamplers.size());

    for (const CombinedTextureSamplerDescriptor& desc : pipelineLayoutDesc.combinedTextureSamplers)
    {
        resourceNames_.push_back(desc.name.c_str());
        combinedSamplerSlots_.push_back(desc.slot.index);
    }
}

bool GLPipelineLayout::BuildCombinedSamplerSlots(
    const PipelineLayoutDescriptor& pipelineLayoutDesc,
    ResourceType                    type,
    const StringView&               name,
    GLuint&                         outFirst,
    std::uint32_t&                  outCount)
{
    if (!pipelineLayoutDesc.combinedTextureSamplers.empty() && (type == ResourceType::Texture || type == ResourceType::Sampler))
    {
        std::size_t firstSlotIndex = combinedSamplerSlots_.size();

        /* Find texture or sampler name in list of combined texture-samplers */
        for (const CombinedTextureSamplerDescriptor& desc : pipelineLayoutDesc.combinedTextureSamplers)
        {
            if ((type == ResourceType::Texture && desc.textureName == name) ||
                (type == ResourceType::Sampler && desc.samplerName == name))
            {
                combinedSamplerSlots_.push_back(desc.slot.index);
            }
        }

        /* Return start index and number of slots if the list has grown */
        const std::size_t numSlots = (combinedSamplerSlots_.size() - firstSlotIndex);
        if (numSlots > 0)
        {
            outFirst = static_cast<GLuint>(firstSlotIndex);
            outCount = static_cast<std::uint32_t>(numSlots);
            return true;
        }
    }
    return false;
}

std::uint32_t GLPipelineLayout::BuildCombinedStaticSamplerSlots(const PipelineLayoutDescriptor& pipelineLayoutDesc, const StringView& name)
{
    if (!pipelineLayoutDesc.combinedTextureSamplers.empty())
    {
        std::size_t firstSlotIndex = staticSamplerSlots_.size();

        /* Find texture or sampler name in list of combined texture-samplers */
        for (const CombinedTextureSamplerDescriptor& desc : pipelineLayoutDesc.combinedTextureSamplers)
        {
            if (desc.samplerName == name)
                staticSamplerSlots_.push_back(desc.slot.index);
        }

        /* Return start index and number of slots if the list has grown */
        const std::size_t numSlots = (staticSamplerSlots_.size() - firstSlotIndex);
        return static_cast<std::uint32_t>(numSlots);
    }
    return 0;
}


} // /namespace LLGL



// ================================================================================
