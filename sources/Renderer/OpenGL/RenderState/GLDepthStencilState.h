/*
 * GLDepthStencilState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_DEPTH_STENCIL_STATE_H
#define LLGL_GL_DEPTH_STENCIL_STATE_H


#include <LLGL/ForwardDecls.h>
#include "../OpenGL.h"
#include <memory>
#include <limits.h>


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

        // Binds only the stencil write mask.
        void BindStencilWriteMaskOnly();

    public:

        // Returns a signed integer of the strict-weak-order (SWO) comparison, and 0 on equality.
        static int CompareSWO(const GLDepthStencilState& lhs, const GLDepthStencilState& rhs);

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
            GLuint  mask        = UINT_MAX;
            GLuint  writeMask   = UINT_MAX;
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
        #if LLGL_SUPPORTS_INDEPENDENT_STENCIL_FACES
        bool                independentStencilFaces_    = false;
        #endif
        bool                referenceDynamic_           = false;
        GLStencilFaceState  stencilFront_;
        GLStencilFaceState  stencilBack_;

};


} // /namespace LLGL


#endif



// ================================================================================
