/*
 * GLDepthStencilState.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_DEPTH_STENCIL_STATE_H
#define LLGL_GL_DEPTH_STENCIL_STATE_H


#include <LLGL/GraphicsPipelineFlags.h>
#include "../OpenGL.h"
#include "../../StaticLimits.h"
#include <memory>


namespace LLGL
{


class GLDepthStencilState;
class GLStateManager;

using GLDepthStencilStateSPtr = std::shared_ptr<GLDepthStencilState>;

class GLDepthStencilState
{

    public:

        GLDepthStencilState() = default;
        GLDepthStencilState(const GLDepthStencilState&) = default;
        GLDepthStencilState& operator = (const GLDepthStencilState&) = default;

        GLDepthStencilState(const DepthDescriptor& depthDesc, const StencilDescriptor& stencilDesc);

        void Bind(GLStateManager& stateMngr);

        // Returns a signed integer of the strict-weak-order (SWO) comparison, and 0 on equality.
        int CompareSWO(const GLDepthStencilState& rhs) const;

    private:

        struct GLStencilFaceState
        {
            static void Convert(GLStencilFaceState& dst, const StencilFaceDescriptor& src);
            static int CompareSWO(const GLStencilFaceState& lhs, const GLStencilFaceState& rhs);

            GLenum  sfail       = GL_KEEP;
            GLenum  dpfail      = GL_KEEP;
            GLenum  dppass      = GL_KEEP;
            GLenum  func        = GL_ALWAYS;
            GLint   ref         = 0;
            GLuint  mask        = ~0;
            GLuint  writeMask   = ~0;
        };

        void BindStencilFaceState(const GLStencilFaceState& state, GLenum face);
        void BindStencilState(const GLStencilFaceState& state);

        // depth states
        bool                depthTestEnabled_           = false;    // glEnable(GL_DEPTH_TEST)
        GLboolean           depthMask_                  = GL_FALSE; // glDepthMask(GL_TRUE)
        GLenum              depthFunc_                  = GL_LESS;

        // stencil states
        bool                stencilTestEnabled_         = false;    // glEnable(GL_STENCIL_TEST)
        bool                independentStencilFaces_    = false;
        GLStencilFaceState  stencilFront_;
        GLStencilFaceState  stencilBack_;

};


} // /namespace LLGL


#endif



// ================================================================================
