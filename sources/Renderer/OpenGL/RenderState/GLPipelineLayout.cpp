/*
 * GLPipelineLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLPipelineLayout.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensionRegistry.h"
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

    #ifdef LLGL_GLEXT_MEMORY_BARRIERS

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
    heapBindings_     { desc.heapBindings                          },
    uniforms_         { desc.uniforms                              },
    barriers_         { ToMemoryBarrierBitfield(desc.barrierFlags) },
    hasNamedBindings_ { HasAnyNamedResourceBindings(desc)          }
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
    #ifdef LLGL_GL_ENABLE_OPENGL2X
    return static_cast<std::uint32_t>(std::max(staticSamplers_.size(), staticSamplersGL2X_.size()));
    #else
    return static_cast<std::uint32_t>(staticSamplers_.size());
    #endif
}

std::uint32_t GLPipelineLayout::GetNumUniforms() const
{
    return static_cast<std::uint32_t>(uniforms_.size());
}

void GLPipelineLayout::BindStaticSamplers(GLStateManager& stateMngr) const
{
    if (!staticSamplerSlots_.empty())
    {
        #ifdef LLGL_GL_ENABLE_OPENGL2X
        if (!HasNativeSamplers())
        {
            for_range(i, staticSamplerSlots_.size())
                stateMngr.BindGL2XSampler(staticSamplerSlots_[i], *staticSamplersGL2X_[i]);
        }
        else
        #endif
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
            #ifdef LLGL_GL_ENABLE_OPENGL2X
            if (!HasNativeSamplers())
                return GLResourceType_GL2XSampler;
            #endif
            return GLResourceType_Sampler;

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
    #ifdef LLGL_GL_ENABLE_OPENGL2X
    if (!HasNativeSamplers())
    {
        staticSamplersGL2X_.reserve(staticSamplerDescs.size());
        for (const StaticSamplerDescriptor& desc : staticSamplerDescs)
        {
            /* Create GL2.x sampler and store slot and name separately */
            GL2XSamplerPtr sampler = MakeUnique<GL2XSampler>();
            sampler->SamplerParameters(desc.sampler);
            staticSamplersGL2X_.push_back(std::move(sampler));
            staticSamplerSlots_.push_back(desc.slot.index);
            resourceNames_.push_back(desc.name);
        }
    }
    else
    #endif
    {
        staticSamplers_.reserve(staticSamplerDescs.size());
        for (const StaticSamplerDescriptor& desc : staticSamplerDescs)
        {
            /* Create GL3+ sampler and store slot and name separately */
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
