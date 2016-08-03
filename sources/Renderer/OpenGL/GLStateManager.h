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

enum class GLBufferTarget
{
    ARRAY_BUFFER = 0,
    ATOMIC_COUNTER_BUFFER,
    COPY_READ_BUFFER,
    COPY_WRITE_BUFFER,
    DISPATCH_INDIRECT_BUFFER,
    DRAW_INDIRECT_BUFFER,
    ELEMENT_ARRAY_BUFFER,
    PIXEL_PACK_BUFFER,
    PIXEL_UNPACK_BUFFER,
    QUERY_BUFFER,
    SHADER_STORAGE_BUFFER,
    TEXTURE_BUFFER,
    TRANSFORM_FEEDBACK_BUFFER,
    UNIFORM_BUFFER,
};

enum class GLTextureTarget
{
    TEXTURE_1D = 0,
    TEXTURE_2D,
    TEXTURE_3D,
    TEXTURE_1D_ARRAY,
    TEXTURE_2D_ARRAY,
    TEXTURE_RECTANGLE,
    TEXTURE_CUBE_MAP,
    TEXTURE_CUBE_MAP_ARRAY,
    TEXTURE_BUFFER,
    TEXTURE_2D_MULTISAMPLE,
    TEXTURE_2D_MULTISAMPLE_ARRAY,
};


class GLStateManager
{

    public:

        GLStateManager();

        /* ----- Common states ----- */

        //! Resets all internal states by querying the values from OpenGL.
        void Reset();

        void Set(GLState state, bool value);
        void Enable(GLState state);
        void Disable(GLState state);

        bool IsEnabled(GLState state) const;

        void Push(GLState state);
        void Pop();
        void Pop(std::size_t count);

        /* ----- Buffer binding ----- */

        void BindBuffer(GLBufferTarget target, GLuint buffer);
        void BindBufferBase(GLBufferTarget target, GLuint index, GLuint buffer);
        void BindVertexArray(GLuint buffer);

        /* ----- Texture binding ----- */

        void BindTexture(GLTextureTarget target, GLuint texture);

    private:

        struct GLRenderState
        {
            GLState state;
            bool    enabled;
        };

        static const std::size_t        numStates           = (static_cast<std::size_t>(GLState::PROGRAM_POINT_SIZE) + 1);
        static const std::size_t        numBufferTargets    = (static_cast<std::size_t>(GLBufferTarget::UNIFORM_BUFFER) + 1);
        static const std::size_t        numTextureTargets   = (static_cast<std::size_t>(GLTextureTarget::TEXTURE_2D_MULTISAMPLE_ARRAY) + 1);

        std::array<bool, numStates>             states_;
        std::array<GLuint, numBufferTargets>    boundBuffers_;
        std::array<GLuint, numTextureTargets>   boundTextures_;
        std::stack<GLRenderState>               stateStack_;

};


} // /namespace LLGL


#endif



// ================================================================================
