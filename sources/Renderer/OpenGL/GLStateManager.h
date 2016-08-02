/*
 * GLStateManager.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_STATE_MANAGER_H__
#define __LLGL_GL_STATE_MANAGER_H__


#include "OpenGL.h"
#include <array>
#include <stack>


namespace LLGL
{


/**
\brief OpenGL boolean state enumeration.
\remarks Similar naming convention is used as in OpenGL API for simplicity.
*/
enum class GLState
{
    BLEND = 0,
    COLOR_LOGIC_OP,
    CULL_FACE,
    DEBUG_OUTPUT,
    DEBUG_OUTPUT_SYNCHRONOUS,
    DEPTH_CLAMP,
    DEPTH_TEST,
    DITHER,
    FRAMEBUFFER_SRGB,
    LINE_SMOOTH,
    MULTISAMPLE,
    POLYGON_OFFSET_FILL,
    POLYGON_OFFSET_LINE,
    POLYGON_OFFSET_POINT,
    POLYGON_SMOOTH,
    PRIMITIVE_RESTART,
    PRIMITIVE_RESTART_FIXED_INDEX,
    RASTERIZER_DISCARD,
    SAMPLE_ALPHA_TO_COVERAGE,
    SAMPLE_ALPHA_TO_ONE,
    SAMPLE_COVERAGE,
    SAMPLE_SHADING,
    SAMPLE_MASK,
    SCISSOR_TEST,
    STENCIL_TEST,
    TEXTURE_CUBE_MAP_SEAMLESS,
    PROGRAM_POINT_SIZE,
};


class GLStateManager
{

    public:

        GLStateManager();

        //! Resets all internal states by querying the values from OpenGL.
        void Reset();

        void Set(GLState state, bool value);
        void Enable(GLState state);
        void Disable(GLState state);

        void Push(GLState state);
        void Pop();
        void Pop(std::size_t count);

    private:

        struct GLRenderState
        {
            GLState state;
            bool    enabled;
        };

        static const std::size_t    numStates = (static_cast<std::size_t>(GLState::PROGRAM_POINT_SIZE) + 1);

        std::array<bool, numStates> states_;
        std::stack<GLRenderState>   stateStack_;

};


} // /namespace LLGL


#endif



// ================================================================================
