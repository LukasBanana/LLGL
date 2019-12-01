/*
 * GLState.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_STATE_H
#define LLGL_GL_STATE_H


#include "../GLProfile.h"


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief OpenGL boolean state enumeration.
\remarks Similar naming convention is used as in OpenGL API for simplicity.
*/
enum class GLState
{
    BLEND = 0,
    CULL_FACE,
    DEPTH_TEST,
    DITHER,
    POLYGON_OFFSET_FILL,
    PRIMITIVE_RESTART_FIXED_INDEX,
    RASTERIZER_DISCARD,
    SAMPLE_ALPHA_TO_COVERAGE,
    SAMPLE_COVERAGE,
    SCISSOR_TEST,
    STENCIL_TEST,
    #ifdef LLGL_OPENGL
    COLOR_LOGIC_OP,
    DEPTH_CLAMP,
    DEBUG_OUTPUT,
    DEBUG_OUTPUT_SYNCHRONOUS,
    FRAMEBUFFER_SRGB,
    LINE_SMOOTH,
    MULTISAMPLE,
    POLYGON_OFFSET_LINE,
    POLYGON_OFFSET_POINT,
    POLYGON_SMOOTH,
    PRIMITIVE_RESTART,
    PROGRAM_POINT_SIZE,
    SAMPLE_ALPHA_TO_ONE,
    SAMPLE_SHADING,
    SAMPLE_MASK,
    TEXTURE_CUBE_MAP_SEAMLESS,
    #endif
    Num,
};

#ifdef LLGL_GL_ENABLE_VENDOR_EXT

enum class GLStateExt
{
    CONSERVATIVE_RASTERIZATION = 0, // either NV or INTEL extension
    Num,
};

#endif

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
    Num,
};

enum class GLFramebufferTarget
{
    FRAMEBUFFER = 0,
    DRAW_FRAMEBUFFER,
    READ_FRAMEBUFFER,
    Num,
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
    Num,
};


/* ----- Structures ----- */

struct GLViewport
{
    GLfloat x;
    GLfloat y;
    GLfloat width;  // default is context width
    GLfloat height; // default is context height
};

struct GLDepthRange
{
    GLclamp_t minDepth;
    GLclamp_t maxDepth;
};

struct GLScissor
{
    GLint   x;
    GLint   y;
    GLsizei width;  // default is context width
    GLsizei height; // default is context height
};

struct GLRenderState
{
    GLenum      drawMode            = GL_TRIANGLES;
    GLenum      primitiveMode       = GL_TRIANGLES;
    GLenum      indexBufferDataType = GL_UNSIGNED_INT;
    GLsizeiptr  indexBufferStride   = 4;
    GLsizeiptr  indexBufferOffset   = 0;
};

struct GLClearValue
{
    GLfloat color[4]    = { 0.0f, 0.0f, 0.0f, 0.0f };
    GLfloat depth       = 1.0f;
    GLint   stencil     = 0;
};

struct GLPixelStore
{
    GLint rowLength     = 0;
    GLint imageHeight   = 0;
    GLint alignment     = 4; // Must be 1, 2, 4, or 8
};


} // /namespace LLGL


#endif



// ================================================================================
