/*
 * GLRasterizerState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_RASTERIZER_STATE_H
#define LLGL_GL_RASTERIZER_STATE_H


#include <LLGL/ForwardDecls.h>
#include "../OpenGL.h"
#include "GLState.h"
#include <memory>
#include <limits>


namespace LLGL
{


class GLRasterizerState;
class GLStateManager;

using GLRasterizerStateSPtr = std::shared_ptr<GLRasterizerState>;

class GLRasterizerState
{

    public:

        GLRasterizerState() = default;
        GLRasterizerState(const GLRasterizerState&) = default;
        GLRasterizerState& operator = (const GLRasterizerState&) = default;

        GLRasterizerState(const RasterizerDescriptor& desc);

        // Binds the entire rasterizer state.
        void Bind(GLStateManager& stateMngr);

        // Binds the front facing only.
        void BindFrontFaceOnly(GLStateManager& stateMngr);

    public:

        // Returns a signed integer of the strict-weak-order (SWO) comparison, and 0 on equality.
        static int CompareSWO(const GLRasterizerState& lhs, const GLRasterizerState& rhs);

    private:

        #ifdef LLGL_OPENGL
        GLenum      polygonMode_            = GL_FILL;
        bool        depthClampEnabled_      = false;    // glEnable(GL_DEPTH_CLAMP)
        #endif

        GLenum      cullFace_               = 0;
        GLenum      frontFace_              = GL_CCW;
        bool        rasterizerDiscard_      = false;    // glEnable(GL_RASTERIZER_DISCARD)
        bool        scissorTestEnabled_     = false;    // glEnable(GL_SCISSOR_TEST)
        bool        multiSampleEnabled_     = false;    // glEnable(GL_MULTISAMPLE)
        bool        lineSmoothEnabled_      = false;    // glEnable(GL_LINE_SMOOTH)
        GLfloat     lineWidth_              = 1.0f;
        bool        polygonOffsetEnabled_   = false;
        GLState     polygonOffsetMode_      = GLState::PolygonOffsetFill;
        GLfloat     polygonOffsetFactor_    = 0.0f;
        GLfloat     polygonOffsetUnits_     = 0.0f;
        GLfloat     polygonOffsetClamp_     = 0.0f;

        #ifdef LLGL_GL_ENABLE_VENDOR_EXT
        bool        conservativeRaster_     = false;    // glEnable(GL_CONSERVATIVE_RASTERIZATION_NV/INTEL)
        #endif

};


} // /namespace LLGL


#endif



// ================================================================================
