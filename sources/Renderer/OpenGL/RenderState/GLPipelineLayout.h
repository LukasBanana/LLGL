/*
 * GLPipelineLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_PIPELINE_LAYOUT_H
#define LLGL_GL_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/DynamicVector.h>
#include "GLResourceType.h"
#include "../Texture/GLSampler.h"
#include "../Texture/GLEmulatedSampler.h"
#include <vector>


namespace LLGL
{


class GLStateManager;

// GL resource binding for heap resources (part of a ResourceHeap).
struct GLHeapResourceBinding
{
    std::string     name;

    // Interface for BindingIterator<>
    ResourceType    type        = ResourceType::Undefined;
    long            bindFlags   = 0;
    long            stageFlags  = 0;
    // /Interface for BindingIterator<>

    GLuint          slot        = 0;
    std::uint32_t   arraySize   = 0;

    // If non-zero, this binding refers to a combined texture-sampler and 'slot' is interpreted
    // as index into the array of combined texture-samplers; see GLPipelineLayout::GetCombinedSamplerSlots().
    std::uint32_t   combiners   = 0;

    inline bool IsSSBO() const
    {
        return (type == ResourceType::Buffer && (bindFlags & (BindFlags::Storage | BindFlags::Sampled)) != 0);
    }
};

// GL resource binding for dynamic resources (*not* part of a ResourceHeap).
struct GLPipelineResourceBinding
{
    GLResourceType  type;
    GLuint          slot;

    // If non-zero, this binding refers to a combined texture-sampler and 'slot' is interpreted
    // as index into the array of combined texture-samplers; see GLPipelineLayout::GetCombinedSamplerSlots().
    std::uint32_t   combiners : 16;

    // Zero-based index for all dynamic storage buffers within the PSO layout.
    std::uint32_t   ssboIndex : 16;

    inline bool IsSSBO() const
    {
        return (type == GLResourceType_Buffer);
    }
};

class GLPipelineLayout final : public PipelineLayout
{

    public:

        #include <LLGL/Backend/PipelineLayout.inl>

    public:

        GLPipelineLayout(const PipelineLayoutDescriptor& desc);

        // Binds the static samplers of this pipeline layout.
        void BindStaticSamplers(GLStateManager& stateMngr) const;

        // Returns the copied list of heap binding descriptors.
        inline const std::vector<GLHeapResourceBinding>& GetHeapBindings() const
        {
            return heapBindings_;
        }

        // Returns the list of dynamic GL resource bindings.
        inline const std::vector<GLPipelineResourceBinding>& GetBindings() const
        {
            return bindings_;
        }

        // Returns the list of static sampler binding slots.
        inline const std::vector<GLuint>& GetStaticSamplerSlots() const
        {
            return staticSamplerSlots_;
        }

        // Returns the list of combined texture-sampler binding slots.
        inline const std::vector<GLuint>& GetCombinedSamplerSlots() const
        {
            return combinedSamplerSlots_;
        }

        // Returns the list of dynamic resource names. Only used by GLShaderBindingLayout.
        inline ArrayView<std::string> GetBindingNames() const
        {
            return ArrayView<std::string>{ resourceNames_.data() + (resourceNames_.size() - bindings_.size()), bindings_.size() };
        }

        // Returns the list of combined texture-sampler names. Only used by GLShaderBindingLayout.
        inline ArrayView<std::string> GetCombinedSamplerNames() const
        {
            return ArrayView<std::string>{ resourceNames_.data(), resourceNames_.size() - bindings_.size() };
        }

        // Returns the copied list of uniform descriptors.
        inline const std::vector<UniformDescriptor>& GetUniforms() const
        {
            return uniforms_;
        }

        // Returns the GLbitfield of memory barriers, used for glMemoryBarrier().
        inline GLbitfield GetBarriersBitfield() const
        {
            return barriers_;
        }

        // Returns true if this pipeline layout needs uniform and block binding. See GLShaderBindingLayout.
        inline bool HasNamedBindings() const
        {
            return hasNamedBindings_;
        }

    private:

        void BuildHeapResourceBindings(const PipelineLayoutDescriptor& pipelineLayoutDesc);
        void BuildDynamicResourceBindings(const PipelineLayoutDescriptor& pipelineLayoutDesc);
        void BuildStaticSamplers(const PipelineLayoutDescriptor& pipelineLayoutDesc);
        void BuildCombinedSamplerNames(const PipelineLayoutDescriptor& pipelineLayoutDesc);

        // Allocates a new combined sampler slot if the input type and name matches the respective 'combinedTextureSamplers' entry.
        bool BuildCombinedSamplerSlots(
            const PipelineLayoutDescriptor& pipelineLayoutDesc,
            ResourceType                    type,
            const StringView&               name,
            GLuint&                         outFirst,
            std::uint32_t&                  outCount
        );

        std::uint32_t BuildCombinedStaticSamplerSlots(const PipelineLayoutDescriptor& pipelineLayoutDesc, const StringView& name);

    private:

        std::vector<std::string>                resourceNames_; // Dynamic resource and static sampler names; Used by GLShaderBindingLayout
        std::vector<GLHeapResourceBinding>      heapBindings_;
        std::vector<GLPipelineResourceBinding>  bindings_;
        std::vector<GLuint>                     staticSamplerSlots_;
        std::vector<GLSamplerSPtr>              staticSamplers_;
        std::vector<GLEmulatedSamplerSPtr>      staticEmulatedSamplers_;
        std::vector<UniformDescriptor>          uniforms_;
        std::vector<GLuint>                     combinedSamplerSlots_;
        const GLbitfield                        barriers_               = 0;
        const bool                              hasNamedBindings_       = false;

};


} // /namespace LLGL


#endif



// ================================================================================
