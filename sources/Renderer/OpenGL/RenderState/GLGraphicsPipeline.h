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
#include "GLBlendState.h"
#include "../Shader/GLShaderProgram.h"
#include <LLGL/GraphicsPipeline.h>
#include <LLGL/RenderSystemFlags.h>
#include <memory>


namespace LLGL
{


struct GraphicsPipelineDescriptor;
class RawBufferIterator;

class GLGraphicsPipeline final : public GraphicsPipeline
{

    public:

        GLGraphicsPipeline(const GraphicsPipelineDescriptor& desc, const RenderingLimits& limits);
        ~GLGraphicsPipeline();

        // Binds this graphics pipeline state with the specified GL state manager.
        void Bind(GLStateManager& stateMngr);

        // Returns the GL mode for drawing commands (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.).
        inline GLenum GetDrawMode() const
        {
            return drawMode_;
        }

    private:

        void BuildStaticStateBuffer(const GraphicsPipelineDescriptor& desc);
        void BuildStaticViewports(std::size_t numViewports, const Viewport* viewports, RawBufferIterator& rawBufferIter);
        void BuildStaticScissors(std::size_t numScissors, const Scissor* scissors, RawBufferIterator& rawBufferIter);

        void SetStaticViewports(GLStateManager& stateMngr, RawBufferIterator& rawBufferIter);
        void SetStaticScissors(GLStateManager& stateMngr, RawBufferIterator& rawBufferIter);

        // shader state
        const GLShaderProgram*  shaderProgram_          = nullptr;

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
        GLbitfield              sampleMask_             = ~0;
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
        GLBlendStateSPtr        blendState_;

        // packed byte buffer for static viewports and scissors
        std::unique_ptr<char[]> staticStateBuffer_;
        GLsizei                 numStaticViewports_     = 0;
        GLsizei                 numStaticScissors_      = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
