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
#include <LLGL/STL/Vector.h>


namespace LLGL
{


class GLStateManager;

// GL resource binding for heap resources (part of a ResourceHeap).
struct GLHeapResourceBinding
{
    STL::string     name;

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
        inline const STL::vector<GLHeapResourceBinding>& GetHeapBindings() const
        {
            return heapBindings_;
        }

        // Returns the list of dynamic GL resource bindings.
        inline const STL::vector<GLPipelineResourceBinding>& GetBindings() const
        {
            return bindings_;
        }

        // Returns the list of static sampler binding slots.
        inline const STL::vector<GLuint>& GetStaticSamplerSlots() const
        {
            return staticSamplerSlots_;
        }

        // Returns the list of combined texture-sampler binding slots.
        inline const STL::vector<GLuint>& GetCombinedSamplerSlots() const
        {
            return combinedSamplerSlots_;
        }

        // Returns the list of dynamic resource names. Only used by GLShaderBindingLayout.
        inline ArrayView<STL::string> GetBindingNames() const
        {
            return ArrayView<STL::string>{ resourceNames_.data() + (resourceNames_.size() - bindings_.size()), bindings_.size() };
        }

        // Returns the list of combined texture-sampler names. Only used by GLShaderBindingLayout.
        inline ArrayView<STL::string> GetCombinedSamplerNames() const
        {
            return ArrayView<STL::string>{ resourceNames_.data(), resourceNames_.size() - bindings_.size() };
        }

        // Returns the copied list of uniform descriptors.
        inline const STL::vector<UniformDescriptor>& GetUniforms() const
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

        STL::vector<STL::string>                resourceNames_; // Dynamic resource and static sampler names; Used by GLShaderBindingLayout
        STL::vector<GLHeapResourceBinding>      heapBindings_;
        STL::vector<GLPipelineResourceBinding>  bindings_;
        STL::vector<GLuint>                     staticSamplerSlots_;
        STL::vector<GLSamplerSPtr>              staticSamplers_;
        STL::vector<GLEmulatedSamplerSPtr>      staticEmulatedSamplers_;
        STL::vector<UniformDescriptor>          uniforms_;
        STL::vector<GLuint>                     combinedSamplerSlots_;
        const GLbitfield                        barriers_               = 0;
        const bool                              hasNamedBindings_       = false;

};


} // /namespace LLGL


#endif



// ================================================================================
