/*
 * GLRasterizerState.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_RASTERIZER_STATE_H
#define LLGL_GL_RASTERIZER_STATE_H


#include <LLGL/ForwardDecls.h>
#include <LLGL/StaticLimits.h>
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

        void Bind(GLStateManager& stateMngr);

        // Returns a signed integer of the strict-weak-order (SWO) comparison, and 0 on equality.
        int CompareSWO(const GLRasterizerState& rhs) const;

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
        GLState     polygonOffsetMode_      = GLState::POLYGON_OFFSET_FILL;
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
