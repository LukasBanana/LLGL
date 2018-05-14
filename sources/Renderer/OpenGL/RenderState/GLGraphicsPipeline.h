/*
 * GLGraphicsPipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_GRAPHICS_PIPELINE_H
#define LLGL_GL_GRAPHICS_PIPELINE_H


#include "../OpenGL.h"
#include "GLStateManager.h"
#include "../Shader/GLShaderProgram.h"
#include <LLGL/GraphicsPipeline.h>
#include <LLGL/RenderSystemFlags.h>
#include <vector>


namespace LLGL
{


class GLGraphicsPipeline : public GraphicsPipeline
{

    public:

        GLGraphicsPipeline(const GraphicsPipelineDescriptor& desc, const RenderingLimits& limits);

        // Binds this graphics pipeline state with the specified GL state manager.
        void Bind(GLStateManager& stateMngr);

        // Returns the GL mode for drawing commands (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.).
        inline GLenum GetDrawMode() const
        {
            return drawMode_;
        }

    private:

        // shader state
        GLShaderProgram*        shaderProgram_          = nullptr;

        // input-assembler state
        GLenum                  drawMode_               = GL_TRIANGLES;
        GLint                   patchVertices_          = 0;

        // depth state
        bool                    depthTestEnabled_       = false;    // glEnable(GL_DEPTH_TEST)
        GLboolean               depthMask_              = false;    // glDepthMask(GL_TRUE)
        GLenum                  depthFunc_              = GL_LESS;

        // stencil state
        bool                    stencilTestEnabled_     = false;    // glEnable(GL_STENCIL_TEST)
        GLStencil               stencilFront_;
        GLStencil               stencilBack_;

        // rasterizer state
        GLenum                  polygonMode_            = GL_FILL;
        GLenum                  cullFace_               = 0;
        GLenum                  frontFace_              = GL_CCW;
        bool                    scissorTestEnabled_     = false;    // glEnable(GL_SCISSOR_TEST)
        bool                    depthClampEnabled_      = false;    // glEnable(GL_DEPTH_CLAMP)
        bool                    multiSampleEnabled_     = false;    // glEnable(GL_MULTISAMPLE)
        bool                    lineSmoothEnabled_      = false;    // glEnable(GL_LINE_SMOOTH)
        GLfloat                 lineWidth_              = 1.0f;
        bool                    polygonOffsetEnabled_   = false;
        GLState                 polygonOffsetMode_      = GLState::POLYGON_OFFSET_FILL;
        GLfloat                 polygonOffsetFactor_    = 0.0f;
        GLfloat                 polygonOffsetUnits_     = 0.0f;
        GLfloat                 polygonOffsetClamp_     = 0.0f;

        #ifdef LLGL_GL_ENABLE_VENDOR_EXT
        bool                    conservativeRaster_     = false;    // glEnable(GL_CONSERVATIVE_RASTERIZATION_NV/INTEL)
        #endif

        // blend state
        bool                    blendEnabled_           = false;
        ColorRGBAf              blendColor_             = { 0.0f, 0.0f, 0.0f, 0.0f };
        bool                    blendColorNeeded_       = false;
        std::vector<GLBlend>    blendStates_;

        // color logic operation state
        bool                    logicOpEnabled_         = false;
        GLenum                  logicOp_                = GL_COPY;

};


} // /namespace LLGL


#endif



// ================================================================================
