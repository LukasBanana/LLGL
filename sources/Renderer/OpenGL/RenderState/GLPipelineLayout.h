/*
 * GLPipelineLayout.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_PIPELINE_LAYOUT_H
#define LLGL_GL_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/Container/ArrayView.h>
#include "GLResourceType.h"
#include "../Texture/GLSampler.h"
#include "../Texture/GL2XSampler.h"
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

        std::uint32_t GetNumHeapBindings() const override;
        std::uint32_t GetNumBindings() const override;
        std::uint32_t GetNumStaticSamplers() const override;
        std::uint32_t GetNumUniforms() const override;

    public:

        GLPipelineLayout(const PipelineLayoutDescriptor& desc);

        // Binds the static samplers of this pipeline layout.
        void BindStaticSamplers(GLStateManager& stateMngr) const;

        // Returns the copied list of binding descriptors.
        inline const std::vector<BindingDescriptor>& GetHeapBindings() const
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

        // Returns true if this pipeline layout needs uniform and block binding. See GLShaderBindingLayout.
        inline bool HasNamedBindings() const
        {
            return hasNamedBindings_;
        }

    private:

        void BuildDynamicResourceBindings(const std::vector<BindingDescriptor>& bindings);
        void BuildStaticSamplers(const std::vector<StaticSamplerDescriptor>& staticSamplers);

    private:

        std::vector<std::string>                resourceNames_; // Dynamic resource and static sampler names; Used by GLShaderBindingLayout
        std::vector<BindingDescriptor>          heapBindings_;
        std::vector<GLPipelineResourceBinding>  bindings_;
        std::vector<GLuint>                     staticSamplerSlots_;
        std::vector<GLSamplerPtr>               staticSamplers_;
        #ifdef LLGL_GL_ENABLE_OPENGL2X
        std::vector<GL2XSamplerPtr>             staticSamplersGL2X_;
        #endif
        std::vector<UniformDescriptor>          uniforms_;
        const bool                              hasNamedBindings_   = false;

};


} // /namespace LLGL


#endif



// ================================================================================
