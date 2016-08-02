/*
 * GLStateManager.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLStateManager.h"


namespace LLGL
{


static const GLenum stateCapsMap[] =
{
    GL_BLEND,
    GL_COLOR_LOGIC_OP,
    GL_CULL_FACE,
    GL_DEBUG_OUTPUT,
    GL_DEBUG_OUTPUT_SYNCHRONOUS,
    GL_DEPTH_CLAMP,
    GL_DEPTH_TEST,
    GL_DITHER,
    GL_FRAMEBUFFER_SRGB,
    GL_LINE_SMOOTH,
    GL_MULTISAMPLE,
    GL_POLYGON_OFFSET_FILL,
    GL_POLYGON_OFFSET_LINE,
    GL_POLYGON_OFFSET_POINT,
    GL_POLYGON_SMOOTH,
    GL_PRIMITIVE_RESTART,
    GL_PRIMITIVE_RESTART_FIXED_INDEX,
    GL_RASTERIZER_DISCARD,
    GL_SAMPLE_ALPHA_TO_COVERAGE,
    GL_SAMPLE_ALPHA_TO_ONE,
    GL_SAMPLE_COVERAGE,
    GL_SAMPLE_SHADING,
    GL_SAMPLE_MASK,
    GL_SCISSOR_TEST,
    GL_STENCIL_TEST,
    GL_TEXTURE_CUBE_MAP_SEAMLESS,
    GL_PROGRAM_POINT_SIZE,
};


GLStateManager::GLStateManager()
{
    std::fill(states_.begin(), states_.end(), false);
}

void GLStateManager::Reset()
{
    /* Query all states from OpenGL */
    for (std::size_t i = 0; i < numStates; ++i)
        states_[i] = (glIsEnabled(stateCapsMap[i]) != GL_FALSE);
}

void GLStateManager::Set(GLState state, bool value)
{
    auto cap = static_cast<GLenum>(state);
    if (states_[cap] != value)
    {
        states_[cap] = value;
        if (value)
            glEnable(stateCapsMap[cap]);
        else
            glDisable(stateCapsMap[cap]);
    }
}

void GLStateManager::Enable(GLState state)
{
    auto cap = static_cast<GLenum>(state);
    if (!states_[cap])
    {
        states_[cap] = true;
        glEnable(stateCapsMap[cap]);
    }
}

void GLStateManager::Disable(GLState state)
{
    auto cap = static_cast<GLenum>(state);
    if (states_[cap])
    {
        states_[cap] = false;
        glDisable(stateCapsMap[cap]);
    }
}

void GLStateManager::Push(GLState state)
{
    stateStack_.push({ state, states_[static_cast<std::size_t>(state)] });
}

void GLStateManager::Pop()
{
    const auto& state = stateStack_.top();
    Set(state.state, state.enabled);
    stateStack_.pop();
}

void GLStateManager::Pop(std::size_t count)
{
    while (count-- > 0)
        Pop();
}


} // /namespace LLGL



// ================================================================================
