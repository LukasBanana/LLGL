/*
 * GLState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_STATE_H
#define LLGL_GL_STATE_H


#include "../Profile/GLProfile.h"


namespace LLGL
{


class GLPipelineLayout;
class GLPipelineState;
class GLBufferWithXFB;

/* ----- Enumerations ----- */

// OpenGL boolean state enumeration.
enum class GLState
{
    Blend = 0,                  // GL_BLEND
    CullFace,                   // GL_CULL_FACE
    DebugOutput,                // GL_DEBUG_OUTPUT
    DebugOutputSynchronous,     // GL_DEBUG_OUTPUT_SYNCHRONOUS
    DepthTest,                  // GL_DEPTH_TEST
    Dither,                     // GL_DITHER
    PolygonOffsetFill,          // GL_POLYGON_OFFSET_FILL
    PrimitiveRestartFixedIndex, // GL_PRIMITIVE_RESTART_FIXED_INDEX
    RasterizerDiscard,          // GL_RASTERIZER_DISCARD
    SampleAlphaToCoverage,      // GL_SAMPLE_ALPHA_TO_COVERAGE
    SampleCoverage,             // GL_SAMPLE_COVERAGE
    ScissorTest,                // GL_SCISSOR_TEST
    StencilTest,                // GL_STENCIL_TEST

    #if LLGL_OPENGL

    ColorLogicOp,               // GL_COLOR_LOGIC_OP
    DepthClamp,                 // GL_DEPTH_CLAMP
    FramebufferSRGB,            // GL_FRAMEBUFFER_SRGB
    LineSmooth,                 // GL_LINE_SMOOTH
    Multisample,                // GL_MULTISAMPLE
    PolygonOffsetLine,          // GL_POLYGON_OFFSET_LINE
    PolygonOffsetPoint,         // GL_POLYGON_OFFSET_POINT
    PolygonSmooth,              // GL_POLYGON_SMOOTH
    PrimitiveRestart,           // GL_PRIMITIVE_RESTART
    ProgramPointSize,           // GL_PROGRAM_POINT_SIZE
    SampleAlphaToOne,           // GL_SAMPLE_ALPHA_TO_ONE
    SampleShading,              // GL_SAMPLE_SHADING
    SampleMask,                 // GL_SAMPLE_MASK
    TextureCubeMapSeamless,     // GL_TEXTURE_CUBE_MAP_SEAMLESS

    #endif // /LLGL_OPENGL

    Num,
};

#ifdef LLGL_GL_ENABLE_VENDOR_EXT

enum class GLStateExt
{
    ConservativeRasterization = 0, // GL_CONSERVATIVE_RASTERIZATION_(NV/INTEL)

    Num,
};

#endif

enum class GLBufferTarget
{
    ArrayBuffer = 0,            // GL_ARRAY_BUFFER
    AtomicCounterBuffer,        // GL_ATOMIC_COUNTER_BUFFER
    CopyReadBuffer,             // GL_COPY_READ_BUFFER
    CopyWriteBuffer,            // GL_COPY_WRITE_BUFFER
    DispatchIndirectBuffer,     // GL_DISPATCH_INDIRECT_BUFFER
    DrawIndirectBuffer,         // GL_DRAW_INDIRECT_BUFFER
    ElementArrayBuffer,         // GL_ELEMENT_ARRAY_BUFFER
    PixelPackBuffer,            // GL_PIXEL_PACK_BUFFER
    PixelUnpackBuffer,          // GL_PIXEL_UNPACK_BUFFER
    QueryBuffer,                // GL_QUERY_BUFFER
    ShaderStorageBuffer,        // GL_SHADER_STORAGE_BUFFER
    TextureBuffer,              // GL_TEXTURE_BUFFER
    TransformFeedbackBuffer,    // GL_TRANSFORM_FEEDBACK_BUFFER
    UniformBuffer,              // GL_UNIFORM_BUFFER

    Num,
};

enum class GLFramebufferTarget
{
    Framebuffer = 0,    // GL_FRAMEBUFFER
    DrawFramebuffer,    // GL_DRAW_FRAMEBUFFER
    ReadFramebuffer,    // GL_READ_FRAMEBUFFER

    Num,
};

enum class GLTextureTarget
{
    Texture1D = 0,              // GL_TEXTURE_1D
    Texture2D,                  // GL_TEXTURE_2D
    Texture3D,                  // GL_TEXTURE_3D
    Texture1DArray,             // GL_TEXTURE_1D_ARRAY
    Texture2DArray,             // GL_TEXTURE_2D_ARRAY
    TextureRectangle,           // GL_TEXTURE_RECTANGLE
    TextureCubeMap,             // GL_TEXTURE_CUBE_MAP
    TextureCubeMapArray,        // GL_TEXTURE_CUBE_MAP_ARRAY
    TextureBuffer,              // GL_TEXTURE_BUFFER
    Texture2DMultisample,       // GL_TEXTURE_2D_MULTISAMPLE
    Texture2DMultisampleArray,  // GL_TEXTURE_2D_MULTISAMPLE_ARRAY

    Num,
};


/* ----- Structures ----- */

// Must be a POD structure.
struct GLViewport
{
    GLfloat x;
    GLfloat y;
    GLfloat width;  // default is context width
    GLfloat height; // default is context height
};

// Must be a POD structure.
struct GLDepthRange
{
    GLclamp_t minDepth;
    GLclamp_t maxDepth;
};

// Must be a POD structure.
struct GLScissor
{
    GLint   x;
    GLint   y;
    GLsizei width;  // default is context width
    GLsizei height; // default is context height
};

struct GLRenderState
{
    GLenum                  drawMode                = GL_TRIANGLES;
    GLenum                  primitiveMode           = GL_TRIANGLES;
    GLenum                  indexBufferDataType     = GL_UNSIGNED_INT;
    GLsizeiptr              indexBufferStride       = 4;
    GLsizeiptr              indexBufferOffset       = 0;
    const GLPipelineLayout* boundPipelineLayout     = nullptr;
    const GLPipelineState*  boundPipelineState      = nullptr;
    GLBufferWithXFB*        boundBufferWithFxb      = nullptr;
    GLbitfield              activeBarriers          = 0;
    GLbitfield              dirtyBarriers           = 0;
};

struct GLPixelStore
{
    GLint rowLength     = 0;
    GLint imageHeight   = 0;
    GLint alignment     = 4; // Must be 1, 2, 4, or 8
};

/*struct GLImageUnit
{
    GLuint  texture = 0;
    GLenum  format  = 0;
    GLenum  access  = 0;
};*/


} // /namespace LLGL


#endif



// ================================================================================
