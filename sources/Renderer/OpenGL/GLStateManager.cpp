/*
 * GLStateManager.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLStateManager.h"
#include "GLExtensions.h"


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

static const GLenum bufferTargetsMap[] =
{
    GL_ARRAY_BUFFER,
    GL_ATOMIC_COUNTER_BUFFER,
    GL_COPY_READ_BUFFER,
    GL_COPY_WRITE_BUFFER,
    GL_DISPATCH_INDIRECT_BUFFER,
    GL_DRAW_INDIRECT_BUFFER,
    GL_ELEMENT_ARRAY_BUFFER,
    GL_PIXEL_PACK_BUFFER,
    GL_PIXEL_UNPACK_BUFFER,
    GL_QUERY_BUFFER,
    GL_SHADER_STORAGE_BUFFER,
    GL_TEXTURE_BUFFER,
    GL_TRANSFORM_FEEDBACK_BUFFER,
    GL_UNIFORM_BUFFER,
};

static const GLenum textureTargetsMap[] =
{
    GL_TEXTURE_1D,
    GL_TEXTURE_2D,
    GL_TEXTURE_3D,
    GL_TEXTURE_1D_ARRAY,
    GL_TEXTURE_2D_ARRAY,
    GL_TEXTURE_RECTANGLE,
    GL_TEXTURE_CUBE_MAP,
    GL_TEXTURE_CUBE_MAP_ARRAY,
    GL_TEXTURE_BUFFER,
    GL_TEXTURE_2D_MULTISAMPLE,
    GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
};


GLStateManager::GLStateManager()
{
    std::fill(states_.begin(), states_.end(), false);
    std::fill(boundBuffers_.begin(), boundBuffers_.end(), 0);
    std::fill(boundTextures_.begin(), boundTextures_.end(), 0);
}

/* ----- Common states ----- */

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

bool GLStateManager::IsEnabled(GLState state) const
{
    return states_[static_cast<std::size_t>(state)];
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

/* ----- Buffer binding ----- */

void GLStateManager::BindBuffer(GLBufferTarget target, GLuint buffer)
{
    /* Only bind buffer if the buffer changed */
    auto targetIdx = static_cast<std::size_t>(target);
    if (boundBuffers_[targetIdx] != buffer)
    {
        boundBuffers_[targetIdx] = buffer;
        glBindBuffer(bufferTargetsMap[targetIdx], buffer);
    }
}

void GLStateManager::BindBufferBase(GLBufferTarget target, GLuint index, GLuint buffer)
{
    /* Always bind buffer with a base index */
    auto targetIdx = static_cast<std::size_t>(target);
    boundBuffers_[targetIdx] = buffer;
    glBindBufferBase(bufferTargetsMap[targetIdx], index, buffer);
}

void GLStateManager::BindVertexArray(GLuint buffer)
{
    /* Always bind vertex array */
    glBindVertexArray(buffer);
    boundBuffers_[static_cast<std::size_t>(GLBufferTarget::ARRAY_BUFFER)] = 0;
    boundBuffers_[static_cast<std::size_t>(GLBufferTarget::ELEMENT_ARRAY_BUFFER)] = 0;
}

/* ----- Texture binding ----- */

void GLStateManager::BindTexture(GLTextureTarget target, GLuint texture)
{
    /* Only bind texutre if the texutre changed */
    auto targetIdx = static_cast<std::size_t>(target);
    if (boundTextures_[targetIdx] != texture)
    {
        boundTextures_[targetIdx] = texture;
        glBindTexture(textureTargetsMap[targetIdx], texture);
    }
}


} // /namespace LLGL



// ================================================================================
