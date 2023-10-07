/*
 * GLStatePool.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_STATE_POOL_H
#define LLGL_GL_STATE_POOL_H


#include "GLState.h"
#include "GLDepthStencilState.h"
#include "GLRasterizerState.h"
#include "GLBlendState.h"
#include "GLPipelineLayout.h"
#include "../Shader/GLShaderBindingLayout.h"
#include "../Shader/GLShaderPipeline.h"
#include "../Shader/GLShader.h"
#include <vector>


namespace LLGL
{


class GLLegacyShader;
class GLSeparableShader;
class GLPipelineCache;

/*
Singleton pool for OpenGL depth-stencil-, rasterizer-, and blend states.
These states are separated from the GLStateManager, because they don't need to exist for every GL context.
*/
class GLStatePool
{

    public:

        GLStatePool(const GLStatePool&) = delete;
        GLStatePool& operator = (const GLStatePool&) = delete;

        // Returns the instance of this pool.
        static GLStatePool& Get();

        // Clear all resource containers of this pool (used by GLRenderSystem).
        void Clear();

        /* ----- Depth-stencil states ----- */

        GLDepthStencilStateSPtr CreateDepthStencilState(const DepthDescriptor& depthDesc, const StencilDescriptor& stencilDesc);
        void ReleaseDepthStencilState(GLDepthStencilStateSPtr&& depthStencilState);

        /* ----- Rasterizer states ----- */

        GLRasterizerStateSPtr CreateRasterizerState(const RasterizerDescriptor& rasterizerDesc);
        void ReleaseRasterizerState(GLRasterizerStateSPtr&& rasterizerState);

        /* ----- Blend states ----- */

        GLBlendStateSPtr CreateBlendState(const BlendDescriptor& blendDesc, std::uint32_t numColorAttachments);
        void ReleaseBlendState(GLBlendStateSPtr&& blendState);

        /* ----- Shader binding layouts ----- */

        GLShaderBindingLayoutSPtr CreateShaderBindingLayout(const GLPipelineLayout& pipelineLayout);
        void ReleaseShaderBindingLayout(GLShaderBindingLayoutSPtr&& shaderBindingLayout);

        /* ----- Shader pipelines ----- */

        GLShaderPipelineSPtr CreateShaderPipeline(
            std::size_t             numShaders,
            Shader* const*          shaders,
            GLShader::Permutation   permutation     = GLShader::PermutationDefault,
            GLPipelineCache*        pipelineCache   = nullptr
        );
        void ReleaseShaderPipeline(GLShaderPipelineSPtr&& shaderPipeline);

    private:

        GLStatePool() = default;

    private:

        std::vector<GLDepthStencilStateSPtr>    depthStencilStates_;
        std::vector<GLRasterizerStateSPtr>      rasterizerStates_;
        std::vector<GLBlendStateSPtr>           blendStates_;
        std::vector<GLShaderBindingLayoutSPtr>  shaderBindingLayouts_;
        std::vector<GLShaderPipelineSPtr>       shaderPipelines_;

};


} // /namespace LLGL


#endif



// ================================================================================
