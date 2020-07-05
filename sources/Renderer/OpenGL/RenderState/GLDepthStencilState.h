/*
 * GLDepthStencilState.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_DEPTH_STENCIL_STATE_H
#define LLGL_GL_DEPTH_STENCIL_STATE_H


#include <LLGL/ForwardDecls.h>
#include <LLGL/StaticLimits.h>
#include "../OpenGL.h"
#include <memory>
#include <limits>


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

        // Binds the entire depth-stencil state.
        void Bind(GLStateManager& stateMngr);

        // Binds only the stencil reference together with the remaining parameters for the glStencilFunc* call.
        void BindStencilRefOnly(GLint ref, GLenum face = GL_FRONT_AND_BACK);

        // Returns a signed integer of the strict-weak-order (SWO) comparison, and 0 on equality.
        int CompareSWO(const GLDepthStencilState& rhs) const;

    private:

        struct GLStencilFaceState
        {
            static void Convert(GLStencilFaceState& dst, const StencilFaceDescriptor& src, bool referenceDynamic);
            static int CompareSWO(const GLStencilFaceState& lhs, const GLStencilFaceState& rhs);

            GLenum  sfail       = GL_KEEP;
            GLenum  dpfail      = GL_KEEP;
            GLenum  dppass      = GL_KEEP;
            GLenum  func        = GL_ALWAYS;
            GLint   ref         = 0;
            GLuint  mask        = std::numeric_limits<GLuint>::max();
            GLuint  writeMask   = std::numeric_limits<GLuint>::max();
        };

    private:

        void BindStencilFaceState(const GLStencilFaceState& state, GLenum face);
        void BindStencilState(const GLStencilFaceState& state);

    private:

        // Depth states
        bool                depthTestEnabled_           = false;    // glEnable(GL_DEPTH_TEST)
        GLboolean           depthMask_                  = GL_FALSE; // glDepthMask(GL_TRUE)
        GLenum              depthFunc_                  = GL_LESS;

        // Stencil states
        bool                stencilTestEnabled_         = false;    // glEnable(GL_STENCIL_TEST)
        bool                independentStencilFaces_    = false;
        bool                referenceDynamic_           = false;
        GLStencilFaceState  stencilFront_;
        GLStencilFaceState  stencilBack_;

};


} // /namespace LLGL


#endif



// ================================================================================
