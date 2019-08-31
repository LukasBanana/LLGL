/*
 * GLGraphicsPipeline.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_GRAPHICS_PIPELINE_H
#define LLGL_GL_GRAPHICS_PIPELINE_H


#include "../OpenGL.h"
#include "GLStateManager.h"
#include "GLDepthStencilState.h"
#include "GLRasterizerState.h"
#include "GLBlendState.h"
#include "../Shader/GLShaderProgram.h"
#include "../Shader/GLShaderBindingLayout.h"
#include <LLGL/GraphicsPipeline.h>
#include <LLGL/RenderSystemFlags.h>
#include <memory>


namespace LLGL
{


struct GraphicsPipelineDescriptor;
class ByteBufferIterator;

class GLGraphicsPipeline final : public GraphicsPipeline
{

    public:

        GLGraphicsPipeline(const GraphicsPipelineDescriptor& desc, const RenderingLimits& limits);
        ~GLGraphicsPipeline();

        // Binds this graphics pipeline state with the specified GL state manager.
        void Bind(GLStateManager& stateMngr);

        // Returns the shader program used for this graphics pipeline.
        inline const GLShaderProgram* GetShaderProgram() const
        {
            return shaderProgram_;
        }

        // Returns the GL mode for drawing commands (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.).
        inline GLenum GetDrawMode() const
        {
            return drawMode_;
        }

    private:

        void BuildStaticStateBuffer(const GraphicsPipelineDescriptor& desc);
        void BuildStaticViewports(std::size_t numViewports, const Viewport* viewports, ByteBufferIterator& byteBufferIter);
        void BuildStaticScissors(std::size_t numScissors, const Scissor* scissors, ByteBufferIterator& byteBufferIter);

        void SetStaticViewports(GLStateManager& stateMngr, ByteBufferIterator& byteBufferIter);
        void SetStaticScissors(GLStateManager& stateMngr, ByteBufferIterator& byteBufferIter);

    private:

        // shader state
        const GLShaderProgram*      shaderProgram_          = nullptr;
        GLShaderBindingLayoutSPtr   shaderBindingLayout_;

        // input-assembler state
        GLenum                      drawMode_               = GL_TRIANGLES;
        GLint                       patchVertices_          = 0;

        // state objects
        GLDepthStencilStateSPtr     depthStencilState_;
        GLRasterizerStateSPtr       rasterizerState_;
        GLBlendStateSPtr            blendState_;

        // packed byte buffer for static viewports and scissors
        std::unique_ptr<char[]>     staticStateBuffer_;
        GLsizei                     numStaticViewports_     = 0;
        GLsizei                     numStaticScissors_      = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
