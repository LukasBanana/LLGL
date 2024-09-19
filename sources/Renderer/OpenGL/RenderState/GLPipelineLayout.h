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

// GL resource binding for dynamic resources (*not* part of a ResourceHeap).
struct GLPipelineResourceBinding
{
    GLResourceType  type;
    GLuint          slot;
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
        inline const DynamicVector<BindingDescriptor>& GetHeapBindings() const
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

        // Returns the list of dynamic resource names. Same list size as GetBindings().
        inline ArrayView<std::string> GetBindingNames() const
        {
            return ArrayView<std::string>{ resourceNames_.data(), bindings_.size() };
        }

        // Returns the list of static sampler names.
        inline ArrayView<std::string> GetStaticSamplerNames() const
        {
            return ArrayView<std::string>{ resourceNames_.data() + bindings_.size(), resourceNames_.size() - bindings_.size() };
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

        void BuildDynamicResourceBindings(const std::vector<BindingDescriptor>& bindingDescs);
        void BuildStaticSamplers(const std::vector<StaticSamplerDescriptor>& staticSamplerDescs);

    private:

        std::vector<std::string>                resourceNames_; // Dynamic resource and static sampler names; Used by GLShaderBindingLayout
        DynamicVector<BindingDescriptor>        heapBindings_;
        std::vector<GLPipelineResourceBinding>  bindings_;
        std::vector<GLuint>                     staticSamplerSlots_;
        std::vector<GLSamplerPtr>               staticSamplers_;
        std::vector<GLEmulatedSamplerPtr>       staticEmulatedSamplers_;
        std::vector<UniformDescriptor>          uniforms_;
        const GLbitfield                        barriers_           = 0;
        const bool                              hasNamedBindings_   = false;

};


} // /namespace LLGL


#endif



// ================================================================================
