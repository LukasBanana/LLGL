/*
 * GLGraphicsPipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_GRAPHICS_PIPELINE_H__
#define __LLGL_GL_GRAPHICS_PIPELINE_H__


#include "../OpenGL.h"
#include "GLStateManager.h"
#include "../Shader/GLShaderProgram.h"
#include <LLGL/GraphicsPipeline.h>
#include <vector>


namespace LLGL
{


class GLGraphicsPipeline : public GraphicsPipeline
{

    public:

        GLGraphicsPipeline(const GraphicsPipelineDescriptor& desc);

        void Bind(GLStateManager& stateMngr);

    private:

        // depth state
        bool                    depthTestEnabled_   = false;    // glEnable(GL_DEPTH_TEST)
        GLboolean               depthMask_          = false;    // glDepthMask(GL_TRUE)
        GLenum                  depthFunc_          = GL_LESS;

        // stencil state
        bool                    stencilTestEnabled_ = false;    // glEnable(GL_STENCIL_TEST)
        GLStencil               stencilFront_;
        GLStencil               stencilBack_;

        // rasterizer state
        GLenum                  polygonMode_        = GL_FILL;
        GLenum                  cullFace_           = 0;
        GLenum                  frontFace_          = GL_CCW;
        bool                    scissorTestEnabled_ = false;    // glEnable(GL_SCISSOR_TEST)
        bool                    depthClampEnabled_  = false;    // glEnable(GL_DEPTH_CLAMP)
        bool                    multiSampleEnabled_ = false;    // glEnable(GL_MULTISAMPLE)
        bool                    lineSmoothEnabled_  = false;    // glEnable(GL_LINE_SMOOTH)

        #ifdef LLGL_GL_ENABLE_EXT
        bool                    conservativeRaster_ = false;    // glEnable(GL_CONSERVATIVE_RASTERIZATION_NV/INTEL)
        #endif

        // blend state
        bool                    blendEnabled_       = false;
        std::vector<GLBlend>    blendStates_;

        // shader state
        GLShaderProgram*        shaderProgram_      = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
