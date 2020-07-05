/*
 * GLGraphicsPSO.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_GRAPHICS_PSO_H
#define LLGL_GL_GRAPHICS_PSO_H


#include "GLPipelineState.h"
#include "GLStateManager.h"
#include "GLDepthStencilState.h"
#include "GLRasterizerState.h"
#include "GLBlendState.h"
#include <LLGL/RenderSystemFlags.h>
#include <memory>


namespace LLGL
{


struct GraphicsPipelineDescriptor;
class ByteBufferIterator;

class GLGraphicsPSO final : public GLPipelineState
{

    public:

        GLGraphicsPSO(const GraphicsPipelineDescriptor& desc, const RenderingLimits& limits);
        ~GLGraphicsPSO();

        // Binds this graphics pipeline state with the specified GL state manager.
        void Bind(GLStateManager& stateMngr) override;

        // Returns the GL mode for drawing commands (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.).
        inline GLenum GetDrawMode() const
        {
            return drawMode_;
        }

        // Returns the GL mode for transform-feedback commands (GL_POINTS, GL_LINES, GL_TRIANGLES).
        inline GLenum GetPrimitiveMode() const
        {
            return primitiveMode_;
        }

    private:

        void BuildStaticStateBuffer(const GraphicsPipelineDescriptor& desc);
        void BuildStaticViewports(std::size_t numViewports, const Viewport* viewports, ByteBufferIterator& byteBufferIter);
        void BuildStaticScissors(std::size_t numScissors, const Scissor* scissors, ByteBufferIterator& byteBufferIter);

        void SetStaticViewports(GLStateManager& stateMngr, ByteBufferIterator& byteBufferIter);
        void SetStaticScissors(GLStateManager& stateMngr, ByteBufferIterator& byteBufferIter);

    private:

        // Input-assembler state
        GLenum                  drawMode_           = GL_TRIANGLES; // for glDraw*
        GLenum                  primitiveMode_      = GL_TRIANGLES; // for glBeginTransformFeedback*
        GLint                   patchVertices_      = 0;

        // State objects
        GLDepthStencilStateSPtr depthStencilState_;
        GLRasterizerStateSPtr   rasterizerState_;
        GLBlendStateSPtr        blendState_;

        // Packed byte buffer for static viewports and scissors
        std::unique_ptr<char[]> staticStateBuffer_;
        GLsizei                 numStaticViewports_ = 0;
        GLsizei                 numStaticScissors_  = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
