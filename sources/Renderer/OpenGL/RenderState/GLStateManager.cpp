/*
 * GLStateManager.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLStateManager.h"
#include "GLRenderPass.h"
#include "GLDepthStencilState.h"
#include "GLRasterizerState.h"
#include "GLBlendState.h"
#include "../Shader/GLShaderProgram.h"
#include "../GLSwapChain.h"
#include "../Buffer/GLBuffer.h"
#include "../Buffer/GLBufferWithXFB.h"
#include "../Texture/GLTexture.h"
#include "../Texture/GLRenderTarget.h"
#include "../Texture/GLEmulatedSampler.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../GLTypes.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/TypeInfo.h>
#include <LLGL/Utils/ForRange.h>
#include <functional>

#ifdef LLGL_OPENGL
#include "../Shader/GLProgramPipeline.h"
#endif


namespace LLGL
{


/*
 * Internal constants
 */

// Maps GLState to <cap> in glEnable, glDisable, glIsEnabled
static const GLenum g_stateCapsEnum[] =
{
    GL_BLEND,
    GL_CULL_FACE,

    #if LLGL_GLEXT_DEBUG
    GL_DEBUG_OUTPUT,
    GL_DEBUG_OUTPUT_SYNCHRONOUS,
    #else // LLGL_GLEXT_DEBUG
    0,
    0,
    #endif // /LLGL_GLEXT_DEBUG

    GL_DEPTH_TEST,
    GL_DITHER,
    GL_POLYGON_OFFSET_FILL,

    #ifdef GL_PRIMITIVE_RESTART_FIXED_INDEX
    GL_PRIMITIVE_RESTART_FIXED_INDEX,
    #else
    0,
    #endif

    #ifdef GL_RASTERIZER_DISCARD
    GL_RASTERIZER_DISCARD,
    #else
    0,
    #endif

    GL_SAMPLE_ALPHA_TO_COVERAGE,
    GL_SAMPLE_COVERAGE,
    GL_SCISSOR_TEST,
    GL_STENCIL_TEST,

    #if LLGL_OPENGL

    GL_COLOR_LOGIC_OP,

    #ifdef GL_DEPTH_CLAMP
    GL_DEPTH_CLAMP,
    #else
    0,
    #endif

    #ifdef GL_FRAMEBUFFER_SRGB
    GL_FRAMEBUFFER_SRGB,
    #else
    0,
    #endif

    GL_LINE_SMOOTH,
    GL_MULTISAMPLE,
    GL_POLYGON_OFFSET_LINE,
    GL_POLYGON_OFFSET_POINT,
    GL_POLYGON_SMOOTH,

    #ifdef GL_PRIMITIVE_RESTART
    GL_PRIMITIVE_RESTART,
    #else
    0,
    #endif

    #ifdef GL_PROGRAM_POINT_SIZE
    GL_PROGRAM_POINT_SIZE,
    #else
    0,
    #endif

    #ifdef GL_SAMPLE_ALPHA_TO_ONE
    GL_SAMPLE_ALPHA_TO_ONE,
    #else
    0,
    #endif

    #ifdef GL_SAMPLE_SHADING
    GL_SAMPLE_SHADING,
    #else
    0,
    #endif

    #ifdef GL_SAMPLE_MASK
    GL_SAMPLE_MASK,
    #else
    0,
    #endif

    #ifdef GL_TEXTURE_CUBE_MAP_SEAMLESS
    GL_TEXTURE_CUBE_MAP_SEAMLESS,
    #else
    0,
    #endif

    #endif // /LLGL_OPENGL
};

// Maps GLBufferTarget to <target> in glBindBuffer, glBindBufferBase
static const GLenum g_bufferTargetsEnum[] =
{
    GL_ARRAY_BUFFER,
    #if !LLGL_GL_ENABLE_OPENGL2X
    GL_ATOMIC_COUNTER_BUFFER,
    GL_COPY_READ_BUFFER,
    GL_COPY_WRITE_BUFFER,
    GL_DISPATCH_INDIRECT_BUFFER,
    GL_DRAW_INDIRECT_BUFFER,
    #else
    0, // GL_ATOMIC_COUNTER_BUFFER
    0, // GL_COPY_READ_BUFFER
    0, // GL_COPY_WRITE_BUFFER
    0, // GL_DISPATCH_INDIRECT_BUFFER
    0, // GL_DRAW_INDIRECT_BUFFER
    #endif
    GL_ELEMENT_ARRAY_BUFFER,
    GL_PIXEL_PACK_BUFFER,
    GL_PIXEL_UNPACK_BUFFER,
    #if !LLGL_GL_ENABLE_OPENGL2X
    GL_QUERY_BUFFER,
    GL_SHADER_STORAGE_BUFFER,
    GL_TEXTURE_BUFFER,
    GL_TRANSFORM_FEEDBACK_BUFFER,
    GL_UNIFORM_BUFFER,
    #else
    0, // GL_QUERY_BUFFER
    0, // GL_SHADER_STORAGE_BUFFER
    0, // GL_TEXTURE_BUFFER
    0, // GL_TRANSFORM_FEEDBACK_BUFFER
    0, // GL_UNIFORM_BUFFER
    #endif
};

#if LLGL_GLEXT_FRAMEBUFFER_OBJECT

// Maps GLFramebufferTarget to <target> in glBindFramebuffer
static const GLenum g_framebufferTargetsEnum[] =
{
    GL_FRAMEBUFFER,
    GL_DRAW_FRAMEBUFFER,
    GL_READ_FRAMEBUFFER,
};

#endif // /LLGL_GLEXT_FRAMEBUFFER_OBJECT

// Maps GLTextureTarget to <target> in glBindTexture
static const GLenum g_textureTargetsEnum[] =
{
    GL_TEXTURE_1D,
    GL_TEXTURE_2D,
    GL_TEXTURE_3D,
    #if !LLGL_GL_ENABLE_OPENGL2X
    GL_TEXTURE_1D_ARRAY,
    GL_TEXTURE_2D_ARRAY,
    GL_TEXTURE_RECTANGLE,
    #else
    0, // GL_TEXTURE_1D_ARRAY
    0, // GL_TEXTURE_2D_ARRAY
    0, // GL_TEXTURE_RECTANGLE
    #endif
    GL_TEXTURE_CUBE_MAP,
    #if !LLGL_GL_ENABLE_OPENGL2X
    GL_TEXTURE_CUBE_MAP_ARRAY,
    GL_TEXTURE_BUFFER,
    GL_TEXTURE_2D_MULTISAMPLE,
    GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
    #else
    0, // GL_TEXTURE_CUBE_MAP_ARRAY
    0, // GL_TEXTURE_BUFFER
    0, // GL_TEXTURE_2D_MULTISAMPLE
    0, // GL_TEXTURE_2D_MULTISAMPLE_ARRAY
    #endif
};

// Maps std::uint32_t to <texture> in glActiveTexture
static const GLenum g_textureLayersEnum[] =
{
    GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3,
    GL_TEXTURE4, GL_TEXTURE5, GL_TEXTURE6, GL_TEXTURE7,
    GL_TEXTURE8, GL_TEXTURE9, GL_TEXTURE10, GL_TEXTURE11,
    GL_TEXTURE12, GL_TEXTURE13, GL_TEXTURE14, GL_TEXTURE15,
    GL_TEXTURE16, GL_TEXTURE17, GL_TEXTURE18, GL_TEXTURE19,
    GL_TEXTURE20, GL_TEXTURE21, GL_TEXTURE22, GL_TEXTURE23,
    GL_TEXTURE24, GL_TEXTURE25, GL_TEXTURE26, GL_TEXTURE27,
    GL_TEXTURE28, GL_TEXTURE29, GL_TEXTURE30, GL_TEXTURE31,
};


/*
 * Internal functions
 */

static constexpr GLuint k_invalidGLID = UINT_MAX;

static void InvalidateBoundGLObject(GLuint& boundId, const GLuint releasedObjectId)
{
    /* Invalidate bound ID by setting it to maximum value */
    if (boundId == releasedObjectId)
        boundId = k_invalidGLID;
}


/*
 * GLStateManager static members
 */

GLStateManager*             GLStateManager::current_;
GLStateManager::GLLimits    GLStateManager::commonLimits_;

struct GLStateManager::GLFramebufferClearState
{
    bool        isDepthMaskInvalidated      = false;
    bool        isStencilMaskInvalidated    = false;
    bool        isColorMaskInvalidated      = false;
    GLboolean   oldDepthMask                = GL_TRUE;
    bool        oldRasterizerDiscardState   = false;
    bool        oldScissorTestState         = false;
};


/*
 * GLStateManager class
 */

GLStateManager::GLStateManager()
{
    /* Make this the active state manager if there is no previous one */
    if (GLStateManager::current_ == nullptr)
        GLStateManager::current_ = this;
}

GLStateManager::~GLStateManager()
{
    /* Clean up reference to this state manager if it's the current one */
    if (GLStateManager::current_ == this)
        GLStateManager::current_ = nullptr;
}

void GLStateManager::SetCurrentFromGLContext(GLContext& context)
{
    current_ = &(context.GetStateManager());
}

void GLStateManager::DetermineExtensionsAndLimits()
{
    DetermineLimits();
    #ifdef LLGL_GL_ENABLE_VENDOR_EXT
    DetermineVendorSpecificExtensions();
    #endif
}

void GLStateManager::ResetFramebufferHeight(GLint height)
{
    /* Store new render-target height */
    framebufferHeight_ = height;

    /* Update viewports */
    //TODO...
}

/* ----- Boolean states ----- */

GLenum GLStateManager::GetGLCapability(GLState state)
{
    return g_stateCapsEnum[static_cast<std::size_t>(state)];
}

void GLStateManager::ClearCache()
{
    /* Query entire context state from current GL context */
    GLGetContextState(contextState_);

    /* Clear all pointers and remaining bits to cached objects */
    ::memset(boundGLTextures_, 0, sizeof(boundGLTextures_));
    ::memset(boundGLEmulatedSamplers_, 0, sizeof(boundGLEmulatedSamplers_));

    boundRenderTarget_          = nullptr;
    indexType16Bits_            = false;
    lastVertexAttribArray_      = 0;
    frontFaceInternal_          = GL_CCW;
    flipViewportYPos_           = false;
    flipFrontFacing_            = false;
    emulateOriginUpperLeft_     = false;
    emulateDepthModeZeroToOne_  = false;
    framebufferHeight_          = 0;
    boundDepthStencilState_     = nullptr;
    boundRasterizerState_       = nullptr;
    boundBlendState_            = nullptr;
    frontFacingDirtyBit_        = false;
}

void GLStateManager::Set(GLState state, bool value)
{
    auto idx = static_cast<std::size_t>(state);
    if (contextState_.capabilities[idx] != value)
    {
        contextState_.capabilities[idx] = value;
        if (value)
            glEnable(g_stateCapsEnum[idx]);
        else
            glDisable(g_stateCapsEnum[idx]);
    }
}

void GLStateManager::Enable(GLState state)
{
    auto idx = static_cast<std::size_t>(state);
    if (!contextState_.capabilities[idx])
    {
        contextState_.capabilities[idx] = true;
        glEnable(g_stateCapsEnum[idx]);
    }
}

void GLStateManager::Disable(GLState state)
{
    auto idx = static_cast<std::size_t>(state);
    if (contextState_.capabilities[idx])
    {
        contextState_.capabilities[idx] = false;
        glDisable(g_stateCapsEnum[idx]);
    }
}

bool GLStateManager::IsEnabled(GLState state) const
{
    auto idx = static_cast<std::size_t>(state);
    return contextState_.capabilities[idx];
}

void GLStateManager::PushState(GLState state)
{
    capabilitiesStack_.push(
        {
            state,
            contextState_.capabilities[static_cast<std::size_t>(state)]
        }
    );
}

void GLStateManager::PopState()
{
    const auto& state = capabilitiesStack_.top();
    {
        Set(state.state, state.enabled);
    }
    capabilitiesStack_.pop();
}

void GLStateManager::PopStates(std::size_t count)
{
    while (count-- > 0)
        PopState();
}

#ifdef LLGL_GL_ENABLE_VENDOR_EXT

void GLStateManager::Set(GLStateExt state, bool value)
{
    auto idx = static_cast<std::size_t>(state);
    auto& val = contextState_.capabilitiesExt[idx];
    if (val.cap != 0 && val.enabled != value)
    {
        val.enabled = value;
        if (value)
            glEnable(val.cap);
        else
            glDisable(val.cap);
    }
}

void GLStateManager::Enable(GLStateExt state)
{
    auto idx = static_cast<std::size_t>(state);
    auto& val = contextState_.capabilitiesExt[idx];
    if (val.cap != 0 && !val.enabled)
    {
        val.enabled = true;
        glEnable(val.cap);
    }
}

void GLStateManager::Disable(GLStateExt state)
{
    auto idx = static_cast<std::size_t>(state);
    auto& val = contextState_.capabilitiesExt[idx];
    if (val.cap != 0 && val.enabled)
    {
        val.enabled = false;
        glDisable(val.cap);
    }
}

bool GLStateManager::IsEnabled(GLStateExt state) const
{
    auto idx = static_cast<std::size_t>(state);
    return contextState_.capabilitiesExt[idx].enabled;
}

#endif // /LLGL_GL_ENABLE_VENDOR_EXT

/* ----- Common states ----- */

//private
bool GLStateManager::NeedsAdjustedViewport() const
{
    return flipViewportYPos_;
}

//private
void GLStateManager::AdjustViewport(GLViewport& outViewport, const GLViewport& inViewport)
{
    outViewport.x       = inViewport.x;
    outViewport.y       = static_cast<GLfloat>(framebufferHeight_) - inViewport.height - inViewport.y;
    outViewport.width   = inViewport.width;
    outViewport.height  = inViewport.height;
}

void GLStateManager::SetViewport(const GLViewport& viewport)
{
    /* Adjust viewport for vertical-flipped screen space origin */
    if (NeedsAdjustedViewport())
    {
        GLViewport adjustedViewport;
        AdjustViewport(adjustedViewport, viewport);
        glViewport(
            static_cast<GLint>(adjustedViewport.x),
            static_cast<GLint>(adjustedViewport.y),
            static_cast<GLsizei>(adjustedViewport.width),
            static_cast<GLsizei>(adjustedViewport.height)
        );
    }
    else
    {
        glViewport(
            static_cast<GLint>(viewport.x),
            static_cast<GLint>(viewport.y),
            static_cast<GLsizei>(viewport.width),
            static_cast<GLsizei>(viewport.height)
        );
    }
}

void GLStateManager::AssertViewportLimit(GLuint first, GLsizei count)
{
    if (!HasExtension(GLExt::ARB_viewport_array))
        LLGL_TRAP_FEATURE_NOT_SUPPORTED("GL_ARB_viewport_array");
    if (static_cast<GLint>(first) + count > limits_.maxViewports)
        LLGL_TRAP("GL_ARB_viewport_array: out of bounds: limit is %d, but %d was specified", limits_.maxViewports, static_cast<int>(first + count));
}

void GLStateManager::SetViewportArray(GLuint first, GLsizei count, const GLViewport* viewports)
{
    #ifdef LLGL_GLEXT_VIEWPORT_ARRAY
    if (first + count > 1)
    {
        AssertViewportLimit(first, count);

        /* Adjust viewports for vertical-flipped screen space origin */
        if (NeedsAdjustedViewport())
        {
            GLViewport adjustedViewports[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];

            for_range(i, count)
                AdjustViewport(adjustedViewports[i], viewports[i]);

            glViewportArrayv(first, count, reinterpret_cast<const GLfloat*>(adjustedViewports));
        }
        else
            glViewportArrayv(first, count, reinterpret_cast<const GLfloat*>(viewports));
    }
    else
    #endif // /LLGL_GLEXT_VIEWPORT_ARRAY
    if (count == 1)
    {
        /* Set as single viewport */
        SetViewport(viewports[0]);
    }
}

void GLStateManager::SetDepthRange(const GLDepthRange& depthRange)
{
    GLProfile::DepthRange(depthRange.minDepth, depthRange.maxDepth);
}

void GLStateManager::SetDepthRangeArray(GLuint first, GLsizei count, const GLDepthRange* depthRanges)
{
    #ifdef LLGL_GLEXT_VIEWPORT_ARRAY
    if (first + count > 1)
    {
        AssertViewportLimit(first, count);
        glDepthRangeArrayv(first, count, reinterpret_cast<const GLdouble*>(depthRanges));
    }
    else
    #endif // /LLGL_GLEXT_VIEWPORT_ARRAY
    if (count == 1)
    {
        /* Set as single depth-range */
        SetDepthRange(depthRanges[0]);
    }
}

//private
void GLStateManager::AdjustScissor(GLScissor& outScissor, const GLScissor& inScissor)
{
    outScissor.x        = inScissor.x;
    outScissor.y        = framebufferHeight_ - inScissor.height - inScissor.y;
    outScissor.width    = inScissor.width;
    outScissor.height   = inScissor.height;
}

void GLStateManager::SetScissor(const GLScissor& scissor)
{
    if (NeedsAdjustedViewport())
    {
        GLScissor adjustedScissor;
        AdjustScissor(adjustedScissor, scissor);
        glScissor(adjustedScissor.x, adjustedScissor.y, adjustedScissor.width, adjustedScissor.height);
    }
    else
        glScissor(scissor.x, scissor.y, scissor.width, scissor.height);
}

void GLStateManager::SetScissorArray(GLuint first, GLsizei count, const GLScissor* scissors)
{
    #if LLGL_GLEXT_VIEWPORT_ARRAY
    if (first + count > 1)
    {
        AssertViewportLimit(first, count);

        /* Adjust viewports for vertical-flipped screen space origin */
        if (NeedsAdjustedViewport())
        {
            GLScissor adjustedScissors[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];

            for_range(i, count)
                AdjustScissor(adjustedScissors[i], scissors[i]);

            glScissorArrayv(first, count, reinterpret_cast<const GLint*>(adjustedScissors));
        }
        else
            glScissorArrayv(first, count, reinterpret_cast<const GLint*>(scissors));
    }
    else
    #endif // /LLGL_GLEXT_VIEWPORT_ARRAY
    if (count == 1)
    {
        /* Set as single scissor box */
        SetScissor(scissors[0]);
    }
}

void GLStateManager::SetClipControl(GLenum origin, GLenum depth)
{
    const bool isOriginUpperLeft = (origin == GL_UPPER_LEFT);

    /* Flip viewport if origin is emulated and set to upper-left corner */
    flipViewportYPos_ = !isOriginUpperLeft;

    #if LLGL_GLEXT_CLIP_CONTROL
    if (HasExtension(GLExt::ARB_clip_control))
    {
        /* Use GL extension to transform clipping space */
        if (contextState_.clipOrigin != origin || contextState_.clipDepthMode != depth)
            glClipControl(origin, depth);
    }
    else
    #endif
    {
        /* Emulate clipping space modification; this has to be addressed by transforming gl_Position in each vertex shader */
        emulateOriginUpperLeft_ = isOriginUpperLeft;
        #if LLGL_GLEXT_CLIP_CONTROL
        emulateDepthModeZeroToOne_ = (depth == GL_ZERO_TO_ONE);
        #else
        emulateDepthModeZeroToOne_ = true;
        #endif

        /* Flip front-facing when emulating upper-left origin */
        FlipFrontFacing(isOriginUpperLeft);
    }

    /* Store clipping state */
    contextState_.clipOrigin    = origin;
    contextState_.clipDepthMode = depth;
}

// <face> parameter must always be 'GL_FRONT_AND_BACK' since GL 3.2+
void GLStateManager::SetPolygonMode(GLenum mode)
{
    #ifdef LLGL_OPENGL
    if (contextState_.polygonMode != mode)
    {
        contextState_.polygonMode = mode;
        glPolygonMode(GL_FRONT_AND_BACK, mode);
    }
    #endif
}

void GLStateManager::SetPolygonOffset(GLfloat factor, GLfloat units, GLfloat clamp)
{
    #if LLGL_GLEXT_POLYGON_OFFSET_CLAMP
    if (HasExtension(GLExt::ARB_polygon_offset_clamp))
    {
        if (contextState_.offsetFactor != factor || contextState_.offsetUnits != units || contextState_.offsetClamp != clamp)
        {
            contextState_.offsetFactor  = factor;
            contextState_.offsetUnits   = units;
            contextState_.offsetClamp   = clamp;
            glPolygonOffsetClamp(factor, units, clamp);
        }
    }
    else
    #endif
    {
        if (contextState_.offsetFactor != factor || contextState_.offsetUnits != units)
        {
            contextState_.offsetFactor  = factor;
            contextState_.offsetUnits   = units;
            glPolygonOffset(factor, units);
        }
    }
}

void GLStateManager::SetCullFace(GLenum face)
{
    if (contextState_.cullFace != face)
    {
        contextState_.cullFace = face;
        glCullFace(face);
    }
}

void GLStateManager::SetFrontFace(GLenum mode)
{
    /* Store actual input front face (without inversion) */
    frontFaceInternal_ = mode;

    /* Check if mode must be inverted */
    if (flipFrontFacing_)
        mode = (mode == GL_CW ? GL_CCW : GL_CW);

    /* Set the internal front face mode */
    SetFrontFaceInternal(mode);
}

void GLStateManager::SetPatchVertices(GLint patchVertices)
{
    #if LLGL_GLEXT_TESSELLATION_SHADER
    if (HasExtension(GLExt::ARB_tessellation_shader))
    {
        if (contextState_.patchVertices != patchVertices)
        {
            contextState_.patchVertices = patchVertices;
            glPatchParameteri(GL_PATCH_VERTICES, patchVertices);
        }
    }
    #endif
}

void GLStateManager::SetLineWidth(GLfloat width)
{
    /* Clamp width silently into limited range */
    width = std::max(limits_.lineWidthRange[0], std::min(width, limits_.lineWidthRange[1]));
    if (contextState_.lineWidth != width)
    {
        contextState_.lineWidth = width;
        glLineWidth(width);
    }
}

void GLStateManager::SetPrimitiveRestartIndex(GLuint index)
{
    #if LLGL_PRIMITIVE_RESTART
    if (HasExtension(GLExt::ARB_compatibility))
    {
        if (contextState_.primitiveRestartIndex != index)
        {
            contextState_.primitiveRestartIndex = index;
            glPrimitiveRestartIndex(index);
        }
    }
    #endif
}

void GLStateManager::SetPixelStorePack(GLint rowLength, GLint imageHeight, GLint alignment)
{
    if (contextState_.pixelStorePack.rowLength != rowLength)
    {
        glPixelStorei(GL_PACK_ROW_LENGTH, rowLength);
        contextState_.pixelStorePack.rowLength = rowLength;
    }
    #ifdef LLGL_OPENGL //TODO: emulate for GLES
    if (contextState_.pixelStorePack.imageHeight != imageHeight)
    {
        glPixelStorei(GL_PACK_IMAGE_HEIGHT, imageHeight);
        contextState_.pixelStorePack.imageHeight = imageHeight;
    }
    #endif
    if (contextState_.pixelStorePack.alignment != alignment)
    {
        glPixelStorei(GL_PACK_ALIGNMENT, alignment);
        contextState_.pixelStorePack.alignment = alignment;
    }
}

void GLStateManager::SetPixelStoreUnpack(GLint rowLength, GLint imageHeight, GLint alignment)
{
    if (contextState_.pixelStoreUnpack.rowLength != rowLength)
    {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
        contextState_.pixelStoreUnpack.rowLength = rowLength;
    }
    if (contextState_.pixelStoreUnpack.imageHeight != imageHeight)
    {
        glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, imageHeight);
        contextState_.pixelStoreUnpack.imageHeight = imageHeight;
    }
    if (contextState_.pixelStoreUnpack.alignment != alignment)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
        contextState_.pixelStoreUnpack.alignment = alignment;
    }
}

/* ----- Depth-stencil states ----- */

void GLStateManager::NotifyDepthStencilStateRelease(GLDepthStencilState* depthStencilState)
{
    if (boundDepthStencilState_ == depthStencilState)
        boundDepthStencilState_ = nullptr;
}

void GLStateManager::BindDepthStencilState(GLDepthStencilState* depthStencilState)
{
    if (depthStencilState != nullptr && depthStencilState != boundDepthStencilState_)
    {
        depthStencilState->Bind(*this);
        boundDepthStencilState_ = depthStencilState;
    }
}

void GLStateManager::SetDepthFunc(GLenum func)
{
    if (contextState_.depthFunc != func)
    {
        contextState_.depthFunc = func;
        glDepthFunc(func);
    }
}

void GLStateManager::SetDepthMask(GLboolean flag)
{
    if (contextState_.depthMask != flag)
    {
        contextState_.depthMask = flag;
        glDepthMask(flag);
    }
}

void GLStateManager::SetStencilRef(GLint ref, GLenum face)
{
    if (boundDepthStencilState_ != nullptr)
        boundDepthStencilState_->BindStencilRefOnly(ref, face);
}

/* ----- Rasterizer states ----- */

void GLStateManager::NotifyRasterizerStateRelease(GLRasterizerState* rasterizerState)
{
    if (boundRasterizerState_ == rasterizerState)
    {
        boundRasterizerState_ = nullptr;
        frontFacingDirtyBit_ = false;
    }
}

void GLStateManager::BindRasterizerState(GLRasterizerState* rasterizerState)
{
    if (rasterizerState != nullptr)
    {
        if (rasterizerState != boundRasterizerState_)
        {
            rasterizerState->Bind(*this);
            boundRasterizerState_ = rasterizerState;
            frontFacingDirtyBit_ = false;
        }
        else if (frontFacingDirtyBit_)
        {
            rasterizerState->BindFrontFaceOnly(*this);
            frontFacingDirtyBit_ = false;
        }
    }
}

/* ----- Blend states ----- */

void GLStateManager::NotifyBlendStateRelease(GLBlendState* blendState)
{
    if (boundBlendState_ == blendState)
        boundBlendState_ = nullptr;
}

void GLStateManager::BindBlendState(GLBlendState* blendState)
{
    if (blendState != nullptr && blendState != boundBlendState_)
    {
        blendState->Bind(*this);
        boundBlendState_ = blendState;
    }
}

void GLStateManager::SetBlendColor(const GLfloat color[4])
{
    if ( color[0] != contextState_.blendColor[0] ||
         color[1] != contextState_.blendColor[1] ||
         color[2] != contextState_.blendColor[2] ||
         color[3] != contextState_.blendColor[3] )
    {
        contextState_.blendColor[0] = color[0];
        contextState_.blendColor[1] = color[1];
        contextState_.blendColor[2] = color[2];
        contextState_.blendColor[3] = color[3];
        glBlendColor(color[0], color[1], color[2], color[3]);
    }
}

void GLStateManager::SetLogicOp(GLenum opcode)
{
    #ifdef LLGL_OPENGL
    if (contextState_.logicOpCode != opcode)
    {
        contextState_.logicOpCode = opcode;
        glLogicOp(opcode);
    }
    #endif
}

/* ----- Buffer ----- */

GLenum GLStateManager::ToGLBufferTarget(GLBufferTarget target)
{
    auto targetIdx = static_cast<std::size_t>(target);
    return g_bufferTargetsEnum[targetIdx];
}

void GLStateManager::BindBuffer(GLBufferTarget target, GLuint buffer)
{
    /* Only bind buffer if the buffer has changed */
    auto targetIdx = static_cast<std::size_t>(target);
    if (contextState_.boundBuffers[targetIdx] != buffer)
    {
        glBindBuffer(g_bufferTargetsEnum[targetIdx], buffer);
        contextState_.boundBuffers[targetIdx] = buffer;
    }
}

void GLStateManager::BindBufferBase(GLBufferTarget target, GLuint index, GLuint buffer)
{
    #if LLGL_GLEXT_UNIFORM_BUFFER_OBJECT
    /* Always bind buffer with a base index */
    auto targetIdx = static_cast<std::size_t>(target);
    glBindBufferBase(g_bufferTargetsEnum[targetIdx], index, buffer);
    contextState_.boundBuffers[targetIdx] = buffer;
    #else // LLGL_GLEXT_UNIFORM_BUFFER_OBJECT
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("GL_ARB_uniform_buffer_object");
    #endif // /LLGL_GLEXT_UNIFORM_BUFFER_OBJECT
}

void GLStateManager::BindBuffersBase(GLBufferTarget target, GLuint first, GLsizei count, const GLuint* buffers)
{
    /* Always bind buffers with a base index */
    auto targetIdx = static_cast<std::size_t>(target);
    auto targetGL = g_bufferTargetsEnum[targetIdx];

    #if LLGL_GLEXT_MULTI_BIND
    if (HasExtension(GLExt::ARB_multi_bind))
    {
        /*
        Bind buffer array, but don't reset the currently bound buffer.
        The spec. of GL_ARB_multi_bind says, that the generic binding point is not modified by this function!
        */
        glBindBuffersBase(targetGL, first, count, buffers);
    }
    else
    #endif // /LLGL_GLEXT_MULTI_BIND
    if (count > 0)
    {
        #if LLGL_GLEXT_UNIFORM_BUFFER_OBJECT
        /* Bind each individual buffer, and store last bound buffer */
        contextState_.boundBuffers[targetIdx] = buffers[count - 1];

        for_range(i, count)
            glBindBufferBase(targetGL, first + i, buffers[i]);
        #else // LLGL_GLEXT_UNIFORM_BUFFER_OBJECT
        LLGL_TRAP_FEATURE_NOT_SUPPORTED("GL_ARB_uniform_buffer_object");
        #endif // /LLGL_GLEXT_UNIFORM_BUFFER_OBJECT
    }
}

void GLStateManager::BindBufferRange(GLBufferTarget target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    #if GL_EXT_transform_feedback && !LLGL_GL_ENABLE_OPENGL2X
    /* Always bind buffer with a base index */
    auto targetIdx = static_cast<std::size_t>(target);
    glBindBufferRange(g_bufferTargetsEnum[targetIdx], index, buffer, offset, size);
    contextState_.boundBuffers[targetIdx] = buffer;
    #else
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("GL_EXT_transform_feedback");
    #endif
}

void GLStateManager::BindBuffersRange(GLBufferTarget target, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizeiptr* sizes)
{
    /* Always bind buffers with a base index */
    auto targetIdx = static_cast<std::size_t>(target);
    auto targetGL = g_bufferTargetsEnum[targetIdx];

    #if LLGL_GLEXT_MULTI_BIND
    if (HasExtension(GLExt::ARB_multi_bind))
    {
        /*
        Bind buffer array, but don't reset the currently bound buffer.
        The spec. of GL_ARB_multi_bind says, that the generic binding point is not modified by this function!
        */
        glBindBuffersRange(targetGL, first, count, buffers, offsets, sizes);
    }
    else
    #endif // /LLGL_GLEXT_MULTI_BIND
    if (count > 0)
    {
        /* Bind each individual buffer, and store last bound buffer */
        contextState_.boundBuffers[targetIdx] = buffers[count - 1];

        #if GL_NV_transform_feedback
        if (HasExtension(GLExt::NV_transform_feedback))
        {
            for_range(i, count)
                glBindBufferRangeNV(targetGL, first + i, buffers[i], offsets[i], sizes[i]);
        }
        else
        #endif // /GL_NV_transform_feedback
        {
            #if GL_EXT_transform_feedback && !LLGL_GL_ENABLE_OPENGL2X
            for_range(i, count)
                glBindBufferRange(targetGL, first + i, buffers[i], offsets[i], sizes[i]);
            #else
            LLGL_TRAP_FEATURE_NOT_SUPPORTED("GL_EXT_transform_feedback");
            #endif
        }
    }
}

// Returns the maximum index value for the specified index data type.
static GLuint GetPrimitiveRestartIndex(bool indexType16Bits)
{
    return (indexType16Bits ? 0xFFFF : 0xFFFFFFFF);
}

void GLStateManager::BindVertexArray(GLuint vertexArray)
{
    #if LLGL_GLEXT_VERTEX_ARRAY_OBJECT

    /* Only bind VAO if it has changed */
    if (contextState_.boundVertexArray != vertexArray)
    {
        /* Bind VAO */
        glBindVertexArray(vertexArray);
        contextState_.boundVertexArray = vertexArray;

        /*
        Always reset index buffer binding
        -> see https://www.opengl.org/wiki/Vertex_Specification#Index_buffers
        */
        contextState_.boundBuffers[static_cast<std::size_t>(GLBufferTarget::ElementArrayBuffer)] = 0;

        if (vertexArray != 0)
        {
            #if LLGL_PRIMITIVE_RESTART

            if (contextState_.boundElementArrayBuffer != 0)
            {
                /* Bind deferred index buffer and enable primitive restart index */
                BindBuffer(GLBufferTarget::ElementArrayBuffer, contextState_.boundElementArrayBuffer);
                Enable(GLState::PrimitiveRestart);
                SetPrimitiveRestartIndex(GetPrimitiveRestartIndex(indexType16Bits_));
            }
            else
            {
                /* Disable primitive restart index if no index buffer is bound */
                Disable(GLState::PrimitiveRestart);
            }

            #else // LLGL_PRIMITIVE_RESTART

            if (contextState_.boundElementArrayBuffer != 0)
            {
                /* Bind deferred index buffer */
                BindBuffer(GLBufferTarget::ElementArrayBuffer, contextState_.boundElementArrayBuffer);
            }

            #endif // /LLGL_PRIMITIVE_RESTART
        }
    }

    #else // LLGL_GLEXT_VERTEX_ARRAY_OBJECT

    LLGL_TRAP_FEATURE_NOT_SUPPORTED("Vertex-Array-Objects");

    #endif // /LLGL_GLEXT_VERTEX_ARRAY_OBJECT
}

void GLStateManager::BindGLBuffer(const GLBuffer& buffer)
{
    BindBuffer(buffer.GetTarget(), buffer.GetID());
}

void GLStateManager::NotifyVertexArrayRelease(GLuint vertexArray)
{
    InvalidateBoundGLObject(contextState_.boundVertexArray, vertexArray);
}

void GLStateManager::BindElementArrayBufferToVAO(GLuint buffer, bool indexType16Bits)
{
    #if LLGL_GLEXT_VERTEX_ARRAY_OBJECT

    /* Always store buffer ID to bind the index buffer the next time "BindVertexArray" is called */
    contextState_.boundElementArrayBuffer   = buffer;
    indexType16Bits_                        = indexType16Bits;

    /* If a valid VAO is currently being bound, bind the specified buffer directly */
    #if LLGL_PRIMITIVE_RESTART

    if (contextState_.boundVertexArray != 0)
    {
        /* Bind index buffer and enable primitive restart index */
        BindBuffer(GLBufferTarget::ElementArrayBuffer, buffer);
        Enable(GLState::PrimitiveRestart);
        SetPrimitiveRestartIndex(GetPrimitiveRestartIndex(indexType16Bits_));
    }
    else
    {
        /* Disable primitive restart index */
        Disable(GLState::PrimitiveRestart);
    }

    #else // LLGL_PRIMITIVE_RESTART

    if (contextState_.boundVertexArray != 0)
    {
        /* Bind index buffer */
        BindBuffer(GLBufferTarget::ElementArrayBuffer, buffer);
    }

    #endif // /LLGL_PRIMITIVE_RESTART

    #else // !LLGL_GLEXT_VERTEX_ARRAY_OBJECT

    /* Bind element array buffer directly (for GL 2.x compatibility) */
    BindBuffer(GLBufferTarget::ElementArrayBuffer, buffer);

    #endif // /LLGL_GLEXT_VERTEX_ARRAY_OBJECT
}

void GLStateManager::PushBoundBuffer(GLBufferTarget target)
{
    bufferStack_.push(
        {
            target,
            contextState_.boundBuffers[static_cast<std::size_t>(target)]
        }
    );
}

void GLStateManager::PopBoundBuffer()
{
    const auto& state = bufferStack_.top();
    {
        if (state.buffer != k_invalidGLID)
            BindBuffer(state.target, state.buffer);
    }
    bufferStack_.pop();
}

void GLStateManager::NotifyBufferRelease(GLuint buffer, GLBufferTarget target)
{
    auto targetIdx = static_cast<std::size_t>(target);
    InvalidateBoundGLObject(contextState_.boundBuffers[targetIdx], buffer);
}

void GLStateManager::NotifyBufferRelease(const GLBuffer& buffer)
{
    GLuint  id          = buffer.GetID();
    long    bindFlags   = buffer.GetBindFlags();

    /* Release buffer ID from all potentially used GL buffer targets */
    if ((bindFlags & BindFlags::VertexBuffer) != 0)
        NotifyBufferRelease(id, GLBufferTarget::ArrayBuffer);
    if ((bindFlags & BindFlags::IndexBuffer) != 0)
        NotifyBufferRelease(id, GLBufferTarget::ElementArrayBuffer);
    if ((bindFlags & BindFlags::ConstantBuffer) != 0)
        NotifyBufferRelease(id, GLBufferTarget::UniformBuffer);
    if ((bindFlags & BindFlags::StreamOutputBuffer) != 0)
        NotifyBufferRelease(id, GLBufferTarget::TransformFeedbackBuffer);
    if ((bindFlags & (BindFlags::Sampled | BindFlags::Storage)) != 0)
        NotifyBufferRelease(id, GLBufferTarget::ShaderStorageBuffer);
    if ((bindFlags & BindFlags::IndirectBuffer) != 0)
    {
        NotifyBufferRelease(id, GLBufferTarget::DrawIndirectBuffer);
        NotifyBufferRelease(id, GLBufferTarget::DispatchIndirectBuffer);
    }

    NotifyBufferRelease(id, GLBufferTarget::CopyReadBuffer);
    NotifyBufferRelease(id, GLBufferTarget::CopyWriteBuffer);
    NotifyBufferRelease(id, buffer.GetTarget());
}

void GLStateManager::DisableVertexAttribArrays(GLuint firstIndex)
{
    /* Disable remaining vertex-attrib-arrays */
    for_subrange(i, firstIndex, lastVertexAttribArray_)
        glDisableVertexAttribArray(i);

    /* Store new highest vertex-attrib-array index */
    lastVertexAttribArray_ = firstIndex;
}

/* ----- Framebuffer ----- */

void GLStateManager::BindGLRenderTarget(GLRenderTarget* renderTarget)
{
    boundRenderTarget_ = renderTarget;
    if (renderTarget)
    {
        BindFramebuffer(GLFramebufferTarget::DrawFramebuffer, renderTarget->GetFramebuffer().GetID());
        SetClipControl(GL_UPPER_LEFT, contextState_.clipDepthMode);
    }
    else
    {
        BindFramebuffer(GLFramebufferTarget::DrawFramebuffer, 0);
        SetClipControl(GL_LOWER_LEFT, contextState_.clipDepthMode);
    }
}

void GLStateManager::BindFramebuffer(GLFramebufferTarget target, GLuint framebuffer)
{
    #if LLGL_GLEXT_FRAMEBUFFER_OBJECT
    /* Only bind framebuffer if the framebuffer has changed */
    auto targetIdx = static_cast<std::size_t>(target);
    if (contextState_.boundFramebuffers[targetIdx] != framebuffer)
    {
        contextState_.boundFramebuffers[targetIdx] = framebuffer;
        glBindFramebuffer(g_framebufferTargetsEnum[targetIdx], framebuffer);
    }
    #endif
}

void GLStateManager::PushBoundFramebuffer(GLFramebufferTarget target)
{
    framebufferStack_.push(
        {
            target,
            contextState_.boundFramebuffers[static_cast<std::size_t>(target)]
        }
    );
}

void GLStateManager::PopBoundFramebuffer()
{
    const auto& state = framebufferStack_.top();
    {
        if (state.framebuffer != k_invalidGLID)
            BindFramebuffer(state.target, state.framebuffer);
    }
    framebufferStack_.pop();
}

void GLStateManager::NotifyFramebufferRelease(GLuint framebuffer)
{
    for (GLuint& boundFramebuffer : contextState_.boundFramebuffers)
        InvalidateBoundGLObject(boundFramebuffer, framebuffer);
}

void GLStateManager::NotifyGLRenderTargetRelease(GLRenderTarget* renderTarget)
{
    if (boundRenderTarget_ == renderTarget)
        boundRenderTarget_ = nullptr;
}

GLRenderTarget* GLStateManager::GetBoundRenderTarget() const
{
    return boundRenderTarget_;
}

/* ----- Renderbuffer ----- */

#if LLGL_GLEXT_FRAMEBUFFER_OBJECT

void GLStateManager::BindRenderbuffer(GLuint renderbuffer)
{
    if (contextState_.boundRenderbuffer != renderbuffer)
    {
        contextState_.boundRenderbuffer = renderbuffer;
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    }
}

void GLStateManager::PushBoundRenderbuffer()
{
    renderbufferStack_.push({ contextState_.boundRenderbuffer });
}

void GLStateManager::PopBoundRenderbuffer()
{
    const auto& state = renderbufferStack_.top();
    {
        if (state.renderbuffer != k_invalidGLID)
            BindRenderbuffer(state.renderbuffer);
    }
    renderbufferStack_.pop();
}

void GLStateManager::DeleteRenderbuffer(GLuint renderbuffer)
{
    if (renderbuffer != 0)
    {
        glDeleteRenderbuffers(1, &renderbuffer);
        InvalidateBoundGLObject(contextState_.boundRenderbuffer, renderbuffer);
    }
}

#else // LLGL_GLEXT_FRAMEBUFFER_OBJECT

void GLStateManager::BindRenderbuffer(GLuint renderbuffer)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("renderbuffers");
}

void GLStateManager::PushBoundRenderbuffer()
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("renderbuffers");
}

void GLStateManager::PopBoundRenderbuffer()
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("renderbuffers");
}

void GLStateManager::DeleteRenderbuffer(GLuint renderbuffer)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("renderbuffers");
}

#endif // /LLGL_GLEXT_FRAMEBUFFER_OBJECT

/* ----- Texture ----- */

GLTextureTarget GLStateManager::GetTextureTarget(const TextureType type)
{
    switch (type)
    {
        case TextureType::Texture1D:        return GLTextureTarget::Texture1D;
        case TextureType::Texture2D:        return GLTextureTarget::Texture2D;
        case TextureType::Texture3D:        return GLTextureTarget::Texture3D;
        case TextureType::TextureCube:      return GLTextureTarget::TextureCubeMap;
        case TextureType::Texture1DArray:   return GLTextureTarget::Texture1DArray;
        case TextureType::Texture2DArray:   return GLTextureTarget::Texture2DArray;
        case TextureType::TextureCubeArray: return GLTextureTarget::TextureCubeMapArray;
        case TextureType::Texture2DMS:      return GLTextureTarget::Texture2DMultisample;
        case TextureType::Texture2DMSArray: return GLTextureTarget::Texture2DMultisampleArray;
        default:                            break;
    }
    LLGL_TRAP("failed to convert texture type to OpenGL texture target");
}

GLenum GLStateManager::ToGLTextureLayer(GLuint layer)
{
    return g_textureLayersEnum[layer];
}

GLenum GLStateManager::ToGLTextureTarget(const GLTextureTarget target)
{
    return g_textureTargetsEnum[static_cast<std::size_t>(target)];
}

void GLStateManager::BindTexture(GLTextureTarget target, GLuint texture)
{
    /* Only bind texutre if the texture has changed */
    auto targetIdx = static_cast<std::size_t>(target);
    GLContextState::TextureLayer* textureLayer = GetActiveTextureLayer();
    if (textureLayer->boundTextures[targetIdx] != texture)
    {
        textureLayer->boundTextures[targetIdx] = texture;
        glBindTexture(g_textureTargetsEnum[targetIdx], texture);
    }
}

void GLStateManager::BindTexture(GLuint layer, GLTextureTarget target, GLuint texture)
{
    #ifdef LLGL_DEBUG
    LLGL_ASSERT_UPPER_BOUND(layer, GLContextState::numTextureLayers);
    #endif

    /* Only bind texutre if the texture has changed */
    auto targetIdx = static_cast<std::size_t>(target);
    GLContextState::TextureLayer& textureLayer = contextState_.textureLayers[layer];
    if (textureLayer.boundTextures[targetIdx] != texture)
    {
        textureLayer.boundTextures[targetIdx] = texture;

        /* Activate specified texture layer and store reference to bound textures array */
        if (contextState_.activeTexture != layer)
        {
            contextState_.activeTexture = layer;
            glActiveTexture(g_textureLayersEnum[layer]);
        }

        /* Bind native GL texture to active layer */
        glBindTexture(g_textureTargetsEnum[targetIdx], texture);
    }
}

void GLStateManager::BindTextures(GLuint first, GLsizei count, const GLTextureTarget* targets, const GLuint* textures)
{
    #if LLGL_GLEXT_MULTI_BIND
    if (HasExtension(GLExt::ARB_multi_bind))
    {
        /* Store bound textures */
        for_range(i, count)
        {
            auto targetIdx = static_cast<std::size_t>(targets[i]);
            contextState_.textureLayers[i + first].boundTextures[targetIdx] = textures[i];
        }

        /*
        Bind all textures at once, but don't reset the currently active texture layer.
        The spec. of GL_ARB_multi_bind states that the active texture slot is not modified by this function.
        see https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_multi_bind.txt
        */
        glBindTextures(first, count, textures);
    }
    else
    #endif // /LLGL_GLEXT_MULTI_BIND
    {
        /* Bind each texture layer individually */
        for_range(i, count)
            BindTexture(first + i, targets[i], textures[i]);
    }
}

void GLStateManager::UnbindTextures(GLuint first, GLsizei count)
{
    #if LLGL_GLEXT_MULTI_BIND
    if (HasExtension(GLExt::ARB_multi_bind))
    {
        /* Reset bound textures */
        for_range(i, count)
        {
            auto& boundTextures = contextState_.textureLayers[i].boundTextures;
            ::memset(boundTextures, 0, sizeof(boundTextures));
        }

        /*
        Unbind all textures at once, but don't reset the currently active texture layer.
        The spec. of GL_ARB_multi_bind states that the active texture slot is not modified by this function.
        see https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_multi_bind.txt
        */
        glBindTextures(first, count, nullptr);
    }
    else
    #endif // /LLGL_GLEXT_MULTI_BIND
    {
        /* Unbind all targets for each texture layer individually */
        for_range(i, count)
        {
            for_range(target, GLContextState::numTextureTargets)
                BindTexture(first + i, static_cast<GLTextureTarget>(target), 0);
        }
    }
}

void GLStateManager::BindImageTexture(GLuint unit, GLint level, GLenum format, GLuint texture)
{
    #if LLGL_GLEXT_SHADER_IMAGE_LOAD_STORE
    if (HasExtension(GLExt::ARB_shader_image_load_store))
    {
        #ifdef LLGL_DEBUG
        LLGL_ASSERT_UPPER_BOUND(unit, limits_.maxImageUnits);
        #endif

        if (texture != 0)
            glBindImageTexture(unit, texture, level, GL_TRUE, 0, GL_READ_WRITE, format);
        else
            glBindImageTexture(unit, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
    }
    else
    #endif // /LLGL_GLEXT_SHADER_IMAGE_LOAD_STORE
    {
        /* Error: extension not supported */
        LLGL_TRAP_FEATURE_NOT_SUPPORTED("GL_ARB_shader_image_load_store");
    }
}

void GLStateManager::BindImageTextures(GLuint first, GLsizei count, const GLenum* formats, const GLuint* textures)
{
    #if LLGL_GLEXT_MULTI_BIND
    if (HasExtension(GLExt::ARB_multi_bind))
    {
        /* Bind all image units at once */
        glBindImageTextures(first, count, textures);
    }
    else
    #endif // /LLGL_GLEXT_MULTI_BIND
    {
        /* Bind image units individually */
        for_range(i, count)
            BindImageTexture(first + static_cast<GLuint>(i), 0, formats[i], textures[i]);
    }
}

void GLStateManager::UnbindImageTextures(GLuint first, GLsizei count)
{
    #if LLGL_GLEXT_MULTI_BIND
    if (HasExtension(GLExt::ARB_multi_bind))
    {
        /* Bind all image units at once */
        glBindImageTextures(first, count, nullptr);
    }
    else
    #endif // /LLGL_GLEXT_MULTI_BIND
    {
        /* Unbind all image units individually */
        for_range(i, count)
            BindImageTexture(first + static_cast<GLuint>(i), 0, 0, 0);
    }
}

void GLStateManager::PushBoundTexture(GLuint layer, GLTextureTarget target)
{
    #ifdef LLGL_DEBUG
    LLGL_ASSERT_UPPER_BOUND(layer, GLContextState::numTextureLayers);
    #endif

    textureState_.push(
        {
            layer,
            target,
            (contextState_.textureLayers[layer].boundTextures[static_cast<std::size_t>(target)])
        }
    );
}

void GLStateManager::PushBoundTexture(GLTextureTarget target)
{
    PushBoundTexture(contextState_.activeTexture, target);
}

void GLStateManager::PopBoundTexture()
{
    const auto& state = textureState_.top();
    {
        if (state.texture != k_invalidGLID)
            BindTexture(state.layer, state.target, state.texture);
    }
    textureState_.pop();
}

void GLStateManager::BindGLTexture(GLTexture& texture)
{
    /* Bind native texture */
    BindTexture(GLStateManager::GetTextureTarget(texture.GetType()), texture.GetID());

    /* Manage reference for emulated sampler binding */
    if (!HasNativeSamplers())
    {
        if (boundGLTextures_[contextState_.activeTexture] != &texture)
        {
            boundGLTextures_[contextState_.activeTexture] = &texture;
            if (const GLEmulatedSampler* emulatedSamplerGL = boundGLEmulatedSamplers_[contextState_.activeTexture])
                texture.BindTexParameters(*emulatedSamplerGL);
        }
    }
}

void GLStateManager::BindGLTexture(GLuint layer, GLTexture& texture)
{
    /* Bind native texture */
    BindTexture(layer, GLStateManager::GetTextureTarget(texture.GetType()), texture.GetID());

    /* Manage reference for emulated sampler binding */
    if (!HasNativeSamplers())
    {
        if (boundGLTextures_[layer] != &texture)
        {
            boundGLTextures_[layer] = &texture;
            if (const GLEmulatedSampler* emulatedSamplerGL = boundGLEmulatedSamplers_[layer])
                texture.BindTexParameters(*emulatedSamplerGL);
        }
    }
}

void GLStateManager::DeleteTexture(GLuint texture, GLTextureTarget target, bool invalidateActiveLayerOnly)
{
    if (texture != 0)
    {
        glDeleteTextures(1, &texture);
        NotifyTextureRelease(texture, target, invalidateActiveLayerOnly);
    }
}

/* ----- Sampler ----- */

#ifdef LLGL_GLEXT_SAMPLER_OBJECTS

void GLStateManager::BindSampler(GLuint layer, GLuint sampler)
{
    #ifdef LLGL_DEBUG
    LLGL_ASSERT_UPPER_BOUND(layer, GLContextState::numTextureLayers);
    #endif

    if (contextState_.boundSamplers[layer] != sampler)
    {
        contextState_.boundSamplers[layer] = sampler;
        glBindSampler(layer, sampler);
    }
}

void GLStateManager::BindSamplers(GLuint first, GLsizei count, const GLuint* samplers)
{
    #if LLGL_GLEXT_MULTI_BIND
    if (count >= 2 && HasExtension(GLExt::ARB_multi_bind))
    {
        /* Store bound samplers */
        for_range(i, count)
            contextState_.boundSamplers[i + first] = samplers[i];

        /* Bind all samplers at once */
        glBindSamplers(first, count, samplers);
    }
    else
    #endif // /LLGL_GLEXT_MULTI_BIND
    {
        /* Bind each sampler individually */
        for_range(i, count)
            BindSampler(first + static_cast<GLuint>(i), samplers[i]);
    }
}

void GLStateManager::NotifySamplerRelease(GLuint sampler)
{
    for (GLuint& boundSampler : contextState_.boundSamplers)
        InvalidateBoundGLObject(boundSampler, sampler);
}

#else // LLGL_GLEXT_SAMPLER_OBJECTS

void GLStateManager::BindSampler(GLuint layer, GLuint sampler)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("GL_ARB_sampler_objects");
}

void GLStateManager::BindSamplers(GLuint first, GLsizei count, const GLuint* samplers)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("GL_ARB_sampler_objects");
}

void GLStateManager::NotifySamplerRelease(GLuint sampler)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("GL_ARB_sampler_objects");
}

#endif // /LLGL_GLEXT_SAMPLER_OBJECTS

void GLStateManager::BindEmulatedSampler(GLuint layer, const GLEmulatedSampler& sampler)
{
    LLGL_ASSERT(!HasNativeSamplers(), "emulated samplers not supported when native samplers are supported");

    #ifdef LLGL_DEBUG
    LLGL_ASSERT_UPPER_BOUND(layer, GLContextState::numTextureLayers);
    #endif

    if (boundGLEmulatedSamplers_[layer] != &sampler)
    {
        boundGLEmulatedSamplers_[layer] = &sampler;
        if (auto texture = boundGLTextures_[layer])
            texture->BindTexParameters(sampler);
    }
}

void GLStateManager::BindCombinedEmulatedSampler(GLuint layer, const GLEmulatedSampler& sampler, GLTexture& texture)
{
    LLGL_ASSERT(!HasNativeSamplers(), "emulated samplers not supported when native samplers are supported");

    #ifdef LLGL_DEBUG
    LLGL_ASSERT_UPPER_BOUND(layer, GLContextState::numTextureLayers);
    #endif

    /* Keep reference to GLTexture for emulated sampler binding */
    boundGLTextures_[layer] = &texture;
    boundGLEmulatedSamplers_[layer] = &sampler;

    /* Update texture parameterf if sampler has changed */
    texture.BindTexParameters(sampler);

    /* Bind native texture */
    BindTexture(layer, GLStateManager::GetTextureTarget(texture.GetType()), texture.GetID());
}

/* ----- Shader program ----- */

void GLStateManager::BindShaderProgram(GLuint program)
{
    if (contextState_.boundProgram != program)
    {
        contextState_.boundProgram = program;
        glUseProgram(program);
    }
}

void GLStateManager::PushBoundShaderProgram()
{
    shaderProgramStack_.push({ contextState_.boundProgram });
}

void GLStateManager::PopBoundShaderProgram()
{
    const auto& state = shaderProgramStack_.top();
    {
        if (state.program != k_invalidGLID)
            BindShaderProgram(state.program);
    }
    shaderProgramStack_.pop();
}

void GLStateManager::NotifyShaderProgramRelease(GLShaderProgram* shaderProgram)
{
    if (shaderProgram != nullptr)
        InvalidateBoundGLObject(contextState_.boundProgram, shaderProgram->GetID());
}

GLuint GLStateManager::GetBoundShaderProgram() const
{
    return contextState_.boundProgram;
}

/* ----- Program pipeline ----- */

#if LLGL_GLEXT_SEPARATE_SHADER_OBJECTS

void GLStateManager::BindProgramPipeline(GLuint pipeline)
{
    #ifdef LLGL_OPENGL
    if (contextState_.boundProgramPipeline != pipeline)
    {
        contextState_.boundProgramPipeline = pipeline;
        glBindProgramPipeline(pipeline);
    }
    #endif
}

void GLStateManager::NotifyProgramPipelineRelease(GLProgramPipeline* programPipeline)
{
    #ifdef LLGL_OPENGL
    if (programPipeline != nullptr)
        InvalidateBoundGLObject(contextState_.boundProgramPipeline, programPipeline->GetID());
    #endif
}

GLuint GLStateManager::GetBoundProgramPipeline() const
{
    return contextState_.boundProgramPipeline;
}

#else // LLGL_GLEXT_SEPARATE_SHADER_OBJECTS

void GLStateManager::BindProgramPipeline(GLuint pipeline)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("GL_ARB_separate_shader_objects");
}

void GLStateManager::NotifyProgramPipelineRelease(GLProgramPipeline* programPipeline)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("GL_ARB_separate_shader_objects");
}

GLuint GLStateManager::GetBoundProgramPipeline() const
{
    return 0; // dummy
}

#endif // /LLGL_GLEXT_SEPARATE_SHADER_OBJECTS

/* ----- Render pass ----- */

void GLStateManager::BindRenderTarget(RenderTarget& renderTarget, GLStateManager** nextStateManager)
{
    /* Bind render target/context */
    if (LLGL::IsInstanceOf<SwapChain>(renderTarget))
    {
        auto& swapChainGL = LLGL_CAST(GLSwapChain&, renderTarget);

        /* Make context current and unbind FBO */
        GLSwapChain::MakeCurrent(&swapChainGL);
        GLStateManager::Get().BindGLRenderTarget(nullptr);

        if (nextStateManager != nullptr)
            *nextStateManager = &(swapChainGL.GetStateManager());
    }
    else
    {
        /* Bind FBO, and notify new render target height */
        auto& renderTargetGL = LLGL_CAST(GLRenderTarget&, renderTarget);
        BindGLRenderTarget(&renderTargetGL);
        ResetFramebufferHeight(static_cast<GLint>(renderTargetGL.GetResolution().height));
    }
}

void GLStateManager::Clear(long flags)
{
    GLFramebufferClearState clearState;
    PrepareRasterizerStateForClear(clearState);

    /* Setup GL clear mask and clear respective buffer */
    GLbitfield mask = 0;
    if ((flags & ClearFlags::Color) != 0)
    {
        PrepareColorMaskForClear(clearState);
        mask |= GL_COLOR_BUFFER_BIT;
    }

    if ((flags & ClearFlags::Depth) != 0)
    {
        PrepareDepthMaskForClear(clearState);
        mask |= GL_DEPTH_BUFFER_BIT;
    }

    if ((flags & ClearFlags::Stencil) != 0)
    {
        PrepareStencilMaskForClear(clearState);
        mask |= GL_STENCIL_BUFFER_BIT;
    }

    /* Clear buffers */
    glClear(mask);

    /* Restore all buffer write masks that were modified as preparation for clear operations */
    RestoreClearState(clearState);
}

#if !LLGL_GL_ENABLE_OPENGL2X

void GLStateManager::ClearBuffers(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    GLFramebufferClearState clearState;
    PrepareRasterizerStateForClear(clearState);

    for (; numAttachments-- > 0; ++attachments)
    {
        if ((attachments->flags & ClearFlags::Color) != 0)
        {
            /* Ensure color mask is enabled */
            PrepareColorMaskForClear(clearState);

            /* Clear color buffer */
            glClearBufferfv(
                GL_COLOR,
                static_cast<GLint>(attachments->colorAttachment),
                attachments->clearValue.color
            );
        }
        else if ((attachments->flags & ClearFlags::DepthStencil) == ClearFlags::DepthStencil)
        {
            /* Ensure depth- and stencil masks are enabled */
            PrepareDepthMaskForClear(clearState);
            PrepareStencilMaskForClear(clearState);

            /* Clear depth and stencil buffer simultaneously */
            glClearBufferfi(
                GL_DEPTH_STENCIL,
                0,
                attachments->clearValue.depth,
                static_cast<GLint>(attachments->clearValue.stencil)
            );
        }
        else if ((attachments->flags & ClearFlags::Depth) != 0)
        {
            /* Ensure depth mask is enabled */
            PrepareDepthMaskForClear(clearState);

            /* Clear only depth buffer */
            glClearBufferfv(GL_DEPTH, 0, &(attachments->clearValue.depth));
        }
        else if ((attachments->flags & ClearFlags::Stencil) != 0)
        {
            /* Ensure stencil mask is enabled */
            PrepareStencilMaskForClear(clearState);

            /* Clear only stencil buffer */
            GLint stencil = static_cast<GLint>(attachments->clearValue.stencil);
            glClearBufferiv(GL_STENCIL, 0, &stencil);
        }
    }

    /* Restore all buffer write masks that were modified as preparation for clear operations */
    RestoreClearState(clearState);
}

#else // !LLGL_GL_ENABLE_OPENGL2X

void GLStateManager::ClearBuffers(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("multi-render-targets");
}

#endif // /!LLGL_GL_ENABLE_OPENGL2X

/* ----- Transform feedback ----- */

void GLStateManager::BindTransformFeedback(GLuint transformFeedback)
{
    #if LLGL_GLEXT_TRNASFORM_FEEDBACK2
    if (contextState_.boundTransformFeedback != transformFeedback)
    {
        contextState_.boundTransformFeedback = transformFeedback;
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback);
    }
    #endif
}

void GLStateManager::NotifyTransformFeedbackRelease(GLBufferWithXFB* bufferWithXfb)
{
    if (bufferWithXfb != nullptr)
        InvalidateBoundGLObject(contextState_.boundTransformFeedback, bufferWithXfb->GetTransformFeedbackID());
}


/*
 * ======= Private: =======
 */

GLContextState::TextureLayer* GLStateManager::GetActiveTextureLayer()
{
    return &(contextState_.textureLayers[contextState_.activeTexture]);
}

void GLStateManager::NotifyTextureRelease(GLuint texture, GLTextureTarget target, bool invalidateActiveLayerOnly)
{
    auto targetIdx = static_cast<std::size_t>(target);
    if (invalidateActiveLayerOnly)
    {
        /* Invalidate GL texture only on active layer (should only be used for internal and temporary textures) */
        InvalidateBoundGLObject(GetActiveTextureLayer()->boundTextures[targetIdx], texture);
    }
    else
    {
        /* Invalidate GL texture on all layers */
        for (GLContextState::TextureLayer& layer : contextState_.textureLayers)
            InvalidateBoundGLObject(layer.boundTextures[targetIdx], texture);
    }
}

void GLStateManager::SetFrontFaceInternal(GLenum mode)
{
    if (contextState_.frontFace != mode)
    {
        contextState_.frontFace = mode;
        glFrontFace(mode);
    }
}

void GLStateManager::FlipFrontFacing(bool isFlipped)
{
    /* Update front face and mark it as outdated for next rastierizer state binding */
    flipFrontFacing_ = isFlipped;
    SetFrontFace(frontFaceInternal_);
    frontFacingDirtyBit_ = true;
}

static void AccumCommonGLLimits(GLStateManager::GLLimits& dst, const GLStateManager::GLLimits& src)
{
    if (dst.maxViewports == 0)
    {
        /* Initialize destination with a copy of source */
        dst = src;
    }
    else
    {
        /* Find smallest limits */
        dst.maxViewports        = std::min(dst.maxViewports, src.maxViewports);
        dst.lineWidthRange[0]   = std::min(dst.lineWidthRange[0], src.lineWidthRange[0]);
        dst.lineWidthRange[1]   = std::min(dst.lineWidthRange[1], src.lineWidthRange[1]);
        dst.maxDebugNameLength  = std::min(dst.maxDebugNameLength, src.maxDebugNameLength);
        dst.maxDebugStackDepth  = std::min(dst.maxDebugStackDepth, src.maxDebugStackDepth);
        dst.maxLabelLength      = std::min(dst.maxLabelLength, src.maxLabelLength);
        dst.maxTextureLayers    = std::min(dst.maxTextureLayers, src.maxTextureLayers);
        dst.maxImageUnits       = std::min(dst.maxImageUnits, src.maxImageUnits);
    }
}

void GLStateManager::DetermineLimits()
{
    /* Get integral limits */
    limits_.maxViewports = GLProfile::GetMaxViewports();

    /* Determine minimal line width range for both aliased and smooth lines */
    GLfloat aliasedLineRange[2] = {};
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, aliasedLineRange);

    #ifdef LLGL_OPENGL
    GLfloat smoothLineRange[2] = {};
    glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, smoothLineRange);

    limits_.lineWidthRange[0] = std::min(aliasedLineRange[0], smoothLineRange[0]);
    limits_.lineWidthRange[1] = std::min(aliasedLineRange[1], smoothLineRange[1]);
    #endif

    /* Get extension specific limits */
    #if LLGL_GLEXT_DEBUG
    if (HasExtension(GLExt::KHR_debug))
    {
        glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH, &limits_.maxDebugNameLength);
        glGetIntegerv(GL_MAX_DEBUG_GROUP_STACK_DEPTH, &limits_.maxDebugStackDepth);
        glGetIntegerv(GL_MAX_LABEL_LENGTH, &limits_.maxLabelLength);
    }
    #endif

    /* Get maximum number of texture layers */
    GLint maxTextureImageUnits = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureImageUnits);
    limits_.maxTextureLayers = std::min(static_cast<GLuint>(GLContextState::numTextureLayers), static_cast<GLuint>(maxTextureImageUnits));

    /* Get maximum number of image units */
    #if LLGL_GLEXT_SHADER_IMAGE_LOAD_STORE
    if (HasExtension(GLExt::ARB_shader_image_load_store))
    {
        GLint maxImageUnits = 0;
        glGetIntegerv(GL_MAX_IMAGE_UNITS, &maxImageUnits);
        limits_.maxImageUnits = std::min(static_cast<GLuint>(GLContextState::numImageUnits), static_cast<GLuint>(maxImageUnits));
    }
    #endif // /LLGL_GLEXT_SHADER_IMAGE_LOAD_STORE

    /* Accumulate common limitations */
    AccumCommonGLLimits(GLStateManager::commonLimits_, limits_);
}

#ifdef LLGL_GL_ENABLE_VENDOR_EXT

void GLStateManager::DetermineVendorSpecificExtensions()
{
    #if GL_NV_conservative_raster || GL_INTEL_conservative_rasterization

    /* Initialize extenstion states */
    auto InitStateExt = [&](GLStateExt state, const GLExt extension, GLenum cap)
    {
        auto idx = static_cast<std::size_t>(state);
        auto& val = contextState_.capabilitiesExt[idx];
        if (val.cap == 0 && HasExtension(extension))
        {
            val.cap     = cap;
            val.enabled = (glIsEnabled(cap) != GL_FALSE);
        }
    };

    #if GL_NV_conservative_raster
    // see https://www.opengl.org/registry/specs/NV/conservative_raster.txt
    InitStateExt(GLStateExt::ConservativeRasterization, GLExt::NV_conservative_raster, GL_CONSERVATIVE_RASTERIZATION_NV);
    #endif

    #if GL_INTEL_conservative_rasterization
    // see https://www.opengl.org/registry/specs/INTEL/conservative_rasterization.txt
    InitStateExt(GLStateExt::ConservativeRasterization, GLExt::INTEL_conservative_rasterization, GL_CONSERVATIVE_RASTERIZATION_INTEL);
    #endif

    #endif
}

#endif

/* ----- Stacks ----- */

void GLStateManager::PrepareRasterizerStateForClear(GLFramebufferClearState& clearState)
{
    /* Temporarily disable GL_RASTERIZER_DISCARD, or glClear* commands will be ignored */
    if (IsEnabled(GLState::RasterizerDiscard))
    {
        Disable(GLState::RasterizerDiscard);
        clearState.oldRasterizerDiscardState = true;
    }

    /* Temporarily disable scissor test */
    if (IsEnabled(GLState::ScissorTest))
    {
        Disable(GLState::ScissorTest);
        clearState.oldScissorTestState = true;
    }
}

void GLStateManager::PrepareColorMaskForClear(GLFramebufferClearState& clearState)
{
    if (!clearState.isColorMaskInvalidated)
    {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        clearState.isColorMaskInvalidated = true;
    }
}

void GLStateManager::PrepareDepthMaskForClear(GLFramebufferClearState& clearState)
{
    if (!clearState.isDepthMaskInvalidated)
    {
        clearState.oldDepthMask = contextState_.depthMask;
        SetDepthMask(GL_TRUE);
        clearState.isDepthMaskInvalidated = true;
    }
}

void GLStateManager::PrepareStencilMaskForClear(GLFramebufferClearState& clearState)
{
    if (!clearState.isStencilMaskInvalidated)
    {
        glStencilMask(0xFFFFFFFF);
        clearState.isStencilMaskInvalidated = true;
    }
}

void GLStateManager::RestoreClearState(const GLFramebufferClearState& clearState)
{
    /* Restore previous depth mask */
    if (clearState.isDepthMaskInvalidated)
        SetDepthMask(clearState.oldDepthMask);

    /* Restore stencil mask from currently bound depth-stencil state */
    if (clearState.isStencilMaskInvalidated && boundDepthStencilState_ != nullptr)
        boundDepthStencilState_->BindStencilWriteMaskOnly();

    /* Restore color mask from currently bound blend state */
    if (clearState.isColorMaskInvalidated && boundBlendState_ != nullptr)
        boundBlendState_->BindColorMaskOnly(*this);

    /* Restore GL_RASTERIZER_DISCARD state */
    if (clearState.oldRasterizerDiscardState)
        Enable(GLState::RasterizerDiscard);

    /* Restore GL_SCISSOR_TEST state */
    if (clearState.oldScissorTestState)
        Enable(GLState::ScissorTest);
}

/* ----- Render pass ----- */

#if !LLGL_GL_ENABLE_OPENGL2X

void GLStateManager::ClearAttachmentsWithRenderPass(
    const GLRenderPass& renderPassGL,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    const ClearValue defaultClearValue;
    GLbitfield mask = renderPassGL.GetClearMask();

    GLFramebufferClearState clearState;
    PrepareRasterizerStateForClear(clearState);

    /* Clear color attachments */
    std::uint32_t clearValueIndex = 0;
    if ((mask & GL_COLOR_BUFFER_BIT) != 0)
        clearValueIndex = ClearColorBuffers(renderPassGL.GetClearColorAttachments(), numClearValues, clearValues, defaultClearValue, clearState);

    /* Clear depth-stencil attachment */
    switch (mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
    {
        case (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT):
        {
            /* Ensure depth- and stencil write masks are enabled */
            PrepareDepthMaskForClear(clearState);
            PrepareStencilMaskForClear(clearState);

            /* Clear depth and stencil buffer simultaneously */
            if (clearValueIndex < numClearValues)
                glClearBufferfi(GL_DEPTH_STENCIL, 0, clearValues[clearValueIndex].depth, static_cast<GLint>(clearValues[clearValueIndex].stencil));
            else
                glClearBufferfi(GL_DEPTH_STENCIL, 0, defaultClearValue.depth, defaultClearValue.stencil);
        }
        break;

        case GL_DEPTH_BUFFER_BIT:
        {
            /* Ensure depth write mask is enabled */
            PrepareDepthMaskForClear(clearState);

            /* Clear only depth buffer */
            if (clearValueIndex < numClearValues)
                glClearBufferfv(GL_DEPTH, 0, &(clearValues[clearValueIndex].depth));
            else
                glClearBufferfv(GL_DEPTH, 0, &(defaultClearValue.depth));
        }
        break;

        case GL_STENCIL_BUFFER_BIT:
        {
            /* Ensure stencil write mask is enabled */
            PrepareStencilMaskForClear(clearState);

            /* Clear only stencil buffer */
            if (clearValueIndex < numClearValues)
            {
                const GLint stencil = static_cast<GLint>(clearValues[clearValueIndex].stencil);
                glClearBufferiv(GL_STENCIL, 0, &stencil);
            }
            else
            {
                const GLint stencil = static_cast<GLint>(defaultClearValue.stencil);
                glClearBufferiv(GL_STENCIL, 0, &stencil);
            }
        }
        break;
    }

    /* Restore all buffer write masks that were modified as preparation for clear operations */
    RestoreClearState(clearState);
}

std::uint32_t GLStateManager::ClearColorBuffers(
    const std::uint8_t*         colorBuffers,
    std::uint32_t               numClearValues,
    const ClearValue*           clearValues,
    const ClearValue&           defaultClearValue,
    GLFramebufferClearState&    clearState)
{
    std::uint32_t clearValueIndex = 0;

    /* Use specified clear values */
    for_range(i, numClearValues)
    {
        /* Check if attachment list has ended */
        if (colorBuffers[i] == 0xFF)
            return clearValueIndex;

        PrepareColorMaskForClear(clearState);
        glClearBufferfv(GL_COLOR, static_cast<GLint>(colorBuffers[i]), clearValues[clearValueIndex].color);
        ++clearValueIndex;
    }

    /* Use default clear values */
    for_subrange(i, numClearValues, LLGL_MAX_NUM_COLOR_ATTACHMENTS)
    {
        /* Check if attachment list has ended */
        if (colorBuffers[i] == 0xFF)
            return clearValueIndex;

        PrepareColorMaskForClear(clearState);
        glClearBufferfv(GL_COLOR, static_cast<GLint>(colorBuffers[i]), defaultClearValue.color);
    }

    return clearValueIndex;
}

#else // !LLGL_GL_ENABLE_OPENGL2X

void GLStateManager::ClearAttachmentsWithRenderPass(
    const GLRenderPass& renderPassGL,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("multi-render-targets");
}

std::uint32_t GLStateManager::ClearColorBuffers(
    const std::uint8_t*         colorBuffers,
    std::uint32_t               numClearValues,
    const ClearValue*           clearValues,
    const ClearValue&           defaultClearValue,
    GLFramebufferClearState&    clearState)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("multi-render-targets");
}

#endif // /!LLGL_GL_ENABLE_OPENGL2X


} // /namespace LLGL



// ================================================================================
