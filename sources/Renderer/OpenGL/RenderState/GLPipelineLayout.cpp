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
            barriers |= GL_SHADER_STORAGE_BARRIER_BIT;
        if ((barrierFlags & BarrierFlags::StorageTexture) != 0)
            barriers |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
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
    heapBindings_     { GetExpandedHeapDescriptors(desc.heapBindings) },
    uniforms_         { desc.uniforms                                 },
    barriers_         { ToMemoryBarrierBitfield(desc.barrierFlags)    },
    hasNamedBindings_ { HasAnyNamedResourceBindings(desc)             }
{
    resourceNames_.reserve(desc.bindings.size() + desc.staticSamplers.size());
    BuildDynamicResourceBindings(desc.bindings);
    BuildStaticSamplers(desc.staticSamplers);
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
                return GLResourceType_SSBO;
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

void GLPipelineLayout::BuildDynamicResourceBindings(const std::vector<BindingDescriptor>& bindingDescs)
{
    bindings_.reserve(bindingDescs.size());
    for (const BindingDescriptor& desc : bindingDescs)
    {
        bindings_.push_back(GLPipelineResourceBinding{ ToGLResourceType(desc), static_cast<GLuint>(desc.slot.index) });
        resourceNames_.push_back(desc.name);
    }
}

void GLPipelineLayout::BuildStaticSamplers(const std::vector<StaticSamplerDescriptor>& staticSamplerDescs)
{
    staticSamplerSlots_.reserve(staticSamplerDescs.size());
    if (!HasNativeSamplers())
    {
        staticEmulatedSamplers_.reserve(staticSamplerDescs.size());
        for (const StaticSamplerDescriptor& desc : staticSamplerDescs)
        {
            /* Create emulated sampler and store slot and name separately */
            GLEmulatedSamplerPtr sampler = MakeUnique<GLEmulatedSampler>();
            sampler->SamplerParameters(desc.sampler);
            staticEmulatedSamplers_.push_back(std::move(sampler));
            staticSamplerSlots_.push_back(desc.slot.index);
            resourceNames_.push_back(desc.name);
        }
    }
    else
    {
        staticSamplers_.reserve(staticSamplerDescs.size());
        for (const StaticSamplerDescriptor& desc : staticSamplerDescs)
        {
            /* Create native sampler (GL 3.3+) and store slot and name separately */
            GLSamplerPtr sampler = MakeUnique<GLSampler>();
            sampler->SamplerParameters(desc.sampler);
            staticSamplers_.push_back(std::move(sampler));
            staticSamplerSlots_.push_back(desc.slot.index);
            resourceNames_.push_back(desc.name);
        }
    }
}


} // /namespace LLGL



// ================================================================================
