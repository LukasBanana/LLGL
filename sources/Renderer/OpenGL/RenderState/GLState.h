/*
 * GLState.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_STATE_H__
#define __LLGL_GL_STATE_H__


#include "../OpenGL.h"
#include <LLGL/ColorRGBA.h>


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
    GLdouble minDepth;
    GLdouble maxDepth;
};

struct GLScissor
{
    GLint   x;
    GLint   y;
    GLsizei width;  // default is context width
    GLsizei height; // default is context height
};

struct GLStencil
{
    GLenum  sfail       = GL_KEEP;
    GLenum  dpfail      = GL_KEEP;
    GLenum  dppass      = GL_KEEP;
    GLenum  func        = GL_ALWAYS;
    GLint   ref         = 0;
    GLuint  mask        = ~0;
    GLuint  writeMask   = ~0;
};

struct GLBlend
{
    GLenum                  srcColor    = GL_ONE;
    GLenum                  destColor   = GL_ZERO;
    GLenum                  srcAlpha    = GL_ONE;
    GLenum                  destAlpha   = GL_ZERO;
    ColorRGBAT<GLboolean>   colorMask   = { GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE };
};


} // /namespace LLGL


#endif



// ================================================================================
