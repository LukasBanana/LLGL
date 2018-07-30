/*
 * GLBlendState.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_BLEND_STATE_H
#define LLGL_GL_BLEND_STATE_H


#include <LLGL/GraphicsPipelineFlags.h>
#include "../OpenGL.h"
#include "../../StaticLimits.h"
#include <memory>


namespace LLGL
{


class GLStateManager;
class GLBlendState;

using GLBlendStateSPtr = std::shared_ptr<GLBlendState>;

class GLBlendState
{

    public:

        GLBlendState() = default;
        GLBlendState(const GLBlendState&) = default;
        GLBlendState& operator = (const GLBlendState&) = default;

        GLBlendState(const BlendDescriptor& desc, std::uint32_t numColorAttachments);

        void Bind(GLStateManager& stateMngr);
        void BindColorMaskOnly(GLStateManager& stateMngr);

        // Returns a signed integer of the strict-weak-order (SWO) comparison, and 0 on equality.
        int CompareSWO(const GLBlendState& rhs) const;

    private:

        struct GLDrawBufferState
        {
            static void Convert(GLDrawBufferState& dst, const BlendTargetDescriptor& src);
            static int CompareSWO(const GLDrawBufferState& lhs, const GLDrawBufferState& rhs);

            GLboolean   blendEnabled    = GL_FALSE;
            GLenum      srcColor        = GL_ONE;
            GLenum      dstColor        = GL_ZERO;
            GLenum      funcColor       = GL_FUNC_ADD;
            GLenum      srcAlpha        = GL_ONE;
            GLenum      dstAlpha        = GL_ZERO;
            GLenum      funcAlpha       = GL_FUNC_ADD;
            GLboolean   colorMask[4]    = { GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE };
        };

        void BindDrawBufferStates(GLStateManager& stateMngr);
        void BindDrawBufferColorMasks(GLStateManager& stateMngr);

        void BindDrawBufferState(const GLDrawBufferState& state);
        void BindIndexedDrawBufferState(const GLDrawBufferState& state, GLuint index);

        void BindDrawBufferColorMask(const GLDrawBufferState& state);
        void BindIndexedDrawBufferColorMask(const GLDrawBufferState& state, GLuint index);

        GLfloat             blendColor_[4]                                  = { 0.0f, 0.0f, 0.0f, 0.0f };
        bool                sampleAlphaToCoverage_                          = false;
        bool                logicOpEnabled_                                 = false;
        GLenum              logicOp_                                        = GL_COPY;
        GLuint              numDrawBuffers_                                 = 0;
        GLDrawBufferState   drawBuffers_[LLGL_MAX_NUM_COLOR_ATTACHMENTS]    = {};

};


} // /namespace LLGL


#endif



// ================================================================================
