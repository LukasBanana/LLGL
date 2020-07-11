/*
 * GLStateManager.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLStateManager.h"
#include "GLRenderPass.h"
#include "GLDepthStencilState.h"
#include "GLRasterizerState.h"
#include "GLBlendState.h"
#include "../GLRenderContext.h"
#include "../Buffer/GLBuffer.h"
#include "../Texture/GLTexture.h"
#include "../Texture/GLRenderTarget.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../GLTypes.h"
#include "../../../Core/Helper.h"
#include "../../../Core/Assertion.h"
#include <functional>


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
    GL_DEPTH_TEST,
    GL_DITHER,
    GL_POLYGON_OFFSET_FILL,
    GL_PRIMITIVE_RESTART_FIXED_INDEX,
    GL_RASTERIZER_DISCARD,
    GL_SAMPLE_ALPHA_TO_COVERAGE,
    GL_SAMPLE_COVERAGE,
    GL_SCISSOR_TEST,
    GL_STENCIL_TEST,
    #ifdef LLGL_OPENGL
    GL_COLOR_LOGIC_OP,
    GL_DEPTH_CLAMP,
    GL_DEBUG_OUTPUT,
    GL_DEBUG_OUTPUT_SYNCHRONOUS,
    GL_FRAMEBUFFER_SRGB,
    GL_LINE_SMOOTH,
    GL_MULTISAMPLE,
    GL_POLYGON_OFFSET_LINE,
    GL_POLYGON_OFFSET_POINT,
    GL_POLYGON_SMOOTH,
    GL_PRIMITIVE_RESTART,
    GL_PROGRAM_POINT_SIZE,
    GL_SAMPLE_ALPHA_TO_ONE,
    GL_SAMPLE_SHADING,
    GL_SAMPLE_MASK,
    GL_TEXTURE_CUBE_MAP_SEAMLESS,
    #endif
};

// Maps GLBufferTarget to <target> in glBindBuffer, glBindBufferBase
static const GLenum g_bufferTargetsEnum[] =
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

// Maps GLFramebufferTarget to <target> in glBindFramebuffer
static const GLenum g_framebufferTargetsEnum[] =
{
    GL_FRAMEBUFFER,
    GL_DRAW_FRAMEBUFFER,
    GL_READ_FRAMEBUFFER,
};

// Maps GLTextureTarget to <target> in glBindTexture
static const GLenum g_textureTargetsEnum[] =
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

static const GLuint g_GLInvalidId = std::numeric_limits<GLuint>::max();

// Global array of null pointers to unbind resource slots
static GLuint g_nullResources[GLStateManager::g_maxNumResourceSlots] = {};


/*
 * Internal functions
 */

static void InvalidateBoundGLObject(GLuint& boundId, const GLuint releasedObjectId)
{
    if (boundId == releasedObjectId)
        boundId = g_GLInvalidId;
}


/*
 * GLStateManager static members
 */

const std::uint32_t         GLStateManager::numTextureLayers;
const std::uint32_t         GLStateManager::numImageUnits;
const std::uint32_t         GLStateManager::numStates;
const std::uint32_t         GLStateManager::numBufferTargets;
const std::uint32_t         GLStateManager::numFramebufferTargets;
const std::uint32_t         GLStateManager::numTextureTargets;

#ifdef LLGL_GL_ENABLE_VENDOR_EXT
const std::uint32_t         GLStateManager::numStatesExt;
#endif // /LLGL_GL_ENABLE_VENDOR_EXT

GLStateManager*             GLStateManager::active_;
GLStateManager::GLLimits    GLStateManager::commonLimits_;


/*
 * GLStateManager class
 */

GLStateManager::GLStateManager()
{
    /* Initialize all states with zero */
    Fill(capabilityState_.values, false);
    Fill(bufferState_.boundBuffers, 0);
    Fill(framebufferState_.boundFramebuffers, 0);
    Fill(samplerState_.boundSamplers, 0);

    for (auto& layer : textureState_.layers)
        Fill(layer.boundTextures, 0);

    SetActiveTextureLayer(0);

    /* Make this the active state manager if there is no previous one */
    if (GLStateManager::active_ == nullptr)
        GLStateManager::active_ = this;
}

void GLStateManager::DetermineExtensionsAndLimits()
{
    DetermineLimits();
    #ifdef LLGL_GL_ENABLE_VENDOR_EXT
    DetermineVendorSpecificExtensions();
    #endif
}

void GLStateManager::NotifyRenderTargetHeight(GLint height)
{
    /* Store new render-target height */
    renderTargetHeight_ = height;

    /* Update viewports */
    //TODO...
}

void GLStateManager::SetGraphicsAPIDependentState(const OpenGLDependentStateDescriptor& stateDesc)
{
    /* Check for necessary updates */
    bool updateFrontFace = (apiDependentState_.invertFrontFace != stateDesc.invertFrontFace);

    /* Store new graphics state */
    apiDependentState_ = stateDesc;

    if (updateFrontFace)
    {
        /* Update front face and reset bound rasterizer state */
        SetFrontFace(commonState_.frontFaceAct);
        boundRasterizerState_ = nullptr;
    }
}

/* ----- Boolean states ----- */

void GLStateManager::Reset()
{
    /* Query all states from OpenGL */
    for (std::size_t i = 0; i < numStates; ++i)
        capabilityState_.values[i] = (glIsEnabled(g_stateCapsEnum[i]) != GL_FALSE);
}

void GLStateManager::Set(GLState state, bool value)
{
    auto idx = static_cast<std::size_t>(state);
    if (capabilityState_.values[idx] != value)
    {
        capabilityState_.values[idx] = value;
        if (value)
            glEnable(g_stateCapsEnum[idx]);
        else
            glDisable(g_stateCapsEnum[idx]);
    }
}

void GLStateManager::Enable(GLState state)
{
    auto idx = static_cast<std::size_t>(state);
    if (!capabilityState_.values[idx])
    {
        capabilityState_.values[idx] = true;
        glEnable(g_stateCapsEnum[idx]);
    }
}

void GLStateManager::Disable(GLState state)
{
    auto idx = static_cast<std::size_t>(state);
    if (capabilityState_.values[idx])
    {
        capabilityState_.values[idx] = false;
        glDisable(g_stateCapsEnum[idx]);
    }
}

bool GLStateManager::IsEnabled(GLState state) const
{
    auto idx = static_cast<std::size_t>(state);
    return capabilityState_.values[idx];
}

void GLStateManager::PushState(GLState state)
{
    capabilityState_.valueStack.push(
        {
            state,
            capabilityState_.values[static_cast<std::size_t>(state)]
        }
    );
}

void GLStateManager::PopState()
{
    const auto& state = capabilityState_.valueStack.top();
    {
        Set(state.state, state.enabled);
    }
    capabilityState_.valueStack.pop();
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
    auto& val = capabilityStateExt_.values[idx];
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
    auto& val = capabilityStateExt_.values[idx];
    if (val.cap != 0 && !val.enabled)
    {
        val.enabled = true;
        glEnable(val.cap);
    }
}

void GLStateManager::Disable(GLStateExt state)
{
    auto idx = static_cast<std::size_t>(state);
    auto& val = capabilityStateExt_.values[idx];
    if (val.cap != 0 && val.enabled)
    {
        val.enabled = false;
        glDisable(val.cap);
    }
}

bool GLStateManager::IsEnabled(GLStateExt state) const
{
    auto idx = static_cast<std::size_t>(state);
    return capabilityStateExt_.values[idx].enabled;
}

#endif

/* ----- Common states ----- */

//private
void GLStateManager::AdjustViewport(GLViewport& viewport)
{
    viewport.y = static_cast<GLfloat>(renderTargetHeight_) - viewport.height - viewport.y;
}

void GLStateManager::SetViewport(GLViewport& viewport)
{
    /* Adjust viewport for vertical-flipped screen space origin */
    if (emulateClipControl_ && !apiDependentState_.originLowerLeft)
        AdjustViewport(viewport);

    glViewport(
        static_cast<GLint>(viewport.x),
        static_cast<GLint>(viewport.y),
        static_cast<GLsizei>(viewport.width),
        static_cast<GLsizei>(viewport.height)
    );
}

void GLStateManager::AssertViewportLimit(GLuint first, GLsizei count)
{
    if (static_cast<GLint>(first) + count > limits_.maxViewports)
    {
        throw std::runtime_error(
            "exceeded limit of viewports/scissors (limits is " + std::to_string(limits_.maxViewports) +
            ", but specified " + std::to_string(first + count) + ")"
        );
    }
}

void GLStateManager::SetViewportArray(GLuint first, GLsizei count, GLViewport* viewports)
{
    #ifdef GL_ARB_viewport_array
    if (first + count > 1)
    {
        AssertViewportLimit(first, count);
        AssertExtViewportArray();

        /* Adjust viewports for vertical-flipped screen space origin */
        if (emulateClipControl_ && !apiDependentState_.originLowerLeft)
        {
            for (GLsizei i = 0; i < count; ++i)
                AdjustViewport(viewports[i]);
        }

        glViewportArrayv(first, count, reinterpret_cast<const GLfloat*>(viewports));
    }
    else
    #endif
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
    #ifdef GL_ARB_viewport_array
    if (first + count > 1)
    {
        AssertViewportLimit(first, count);
        AssertExtViewportArray();

        glDepthRangeArrayv(first, count, reinterpret_cast<const GLdouble*>(depthRanges));
    }
    else
    #endif
    if (count == 1)
    {
        /* Set as single depth-range */
        SetDepthRange(depthRanges[0]);
    }
}

//private
void GLStateManager::AdjustScissor(GLScissor& scissor)
{
    scissor.y = renderTargetHeight_ - scissor.height - scissor.y;
}

void GLStateManager::SetScissor(GLScissor& scissor)
{
    if (emulateClipControl_)
        AdjustScissor(scissor);

    glScissor(scissor.x, scissor.y, scissor.width, scissor.height);
}

void GLStateManager::SetScissorArray(GLuint first, GLsizei count, GLScissor* scissors)
{
    #ifdef GL_ARB_viewport_array
    if (first + count > 1)
    {
        AssertViewportLimit(first, count);
        AssertExtViewportArray();

        /* Adjust viewports for vertical-flipped screen space origin */
        if (emulateClipControl_ && !apiDependentState_.originLowerLeft)
        {
            for (GLsizei i = 0; i < count; ++i)
                AdjustScissor(scissors[0]);
        }

        glScissorArrayv(first, count, reinterpret_cast<const GLint*>(scissors));
    }
    else
    #endif
    if (count == 1)
    {
        /* Set as single scissor box */
        SetScissor(scissors[0]);
    }
}

void GLStateManager::SetClipControl(GLenum origin, GLenum depth)
{
    #if 0
    if (HasExtension(GLExt::ARB_clip_control))
        glClipControl(origin, depth);
    else
        emulateClipControl_ = (origin == GL_UPPER_LEFT);
    #else
    emulateClipControl_ = (origin == GL_UPPER_LEFT);
    #endif
}

// <face> parameter must always be 'GL_FRONT_AND_BACK' since GL 3.2+
void GLStateManager::SetPolygonMode(GLenum mode)
{
    #ifdef LLGL_OPENGL
    if (commonState_.polygonMode != mode)
    {
        commonState_.polygonMode = mode;
        glPolygonMode(GL_FRONT_AND_BACK, mode);
    }
    #endif
}

void GLStateManager::SetPolygonOffset(GLfloat factor, GLfloat units, GLfloat clamp)
{
    #ifdef GL_ARB_polygon_offset_clamp
    if (HasExtension(GLExt::ARB_polygon_offset_clamp))
    {
        if (commonState_.offsetFactor != factor || commonState_.offsetUnits != units || commonState_.offsetClamp != clamp)
        {
            commonState_.offsetFactor   = factor;
            commonState_.offsetUnits    = units;
            commonState_.offsetClamp    = clamp;
            glPolygonOffsetClamp(factor, units, clamp);
        }
    }
    else
    #endif
    {
        if (commonState_.offsetFactor != factor || commonState_.offsetUnits != units)
        {
            commonState_.offsetFactor   = factor;
            commonState_.offsetUnits    = units;
            glPolygonOffset(factor, units);
        }
    }
}

void GLStateManager::SetCullFace(GLenum face)
{
    if (commonState_.cullFace != face)
    {
        commonState_.cullFace = face;
        glCullFace(face);
    }
}

void GLStateManager::SetFrontFace(GLenum mode)
{
    /* Store actual input front face (without inversion) */
    commonState_.frontFaceAct = mode;

    /* Check if mode must be inverted */
    if (apiDependentState_.invertFrontFace)
        mode = (mode == GL_CW ? GL_CCW : GL_CW);

    /* Set front face */
    if (commonState_.frontFace != mode)
    {
        commonState_.frontFace = mode;
        glFrontFace(mode);
    }
}

void GLStateManager::SetPatchVertices(GLint patchVertices)
{
    #ifdef LLGL_GLEXT_TESSELLATION_SHADER
    if (commonState_.patchVertices != patchVertices)
    {
        commonState_.patchVertices = patchVertices;
        glPatchParameteri(GL_PATCH_VERTICES, patchVertices);
    }
    #endif
}

void GLStateManager::SetLineWidth(GLfloat width)
{
    /* Clamp width silently into limited range */
    width = std::max(limits_.lineWidthRange[0], std::min(width, limits_.lineWidthRange[1]));
    if (commonState_.lineWidth != width)
    {
        commonState_.lineWidth = width;
        glLineWidth(width);
    }
}

void GLStateManager::SetPrimitiveRestartIndex(GLuint index)
{
    #ifdef LLGL_PRIMITIVE_RESTART
    if (HasExtension(GLExt::ARB_compatibility))
    {
        if (commonState_.primitiveRestartIndex != index)
        {
            commonState_.primitiveRestartIndex = index;
            glPrimitiveRestartIndex(index);
        }
    }
    #endif
}

void GLStateManager::SetPixelStorePack(GLint rowLength, GLint imageHeight, GLint alignment)
{
    if (pixelStorePack_.rowLength != rowLength)
    {
        glPixelStorei(GL_PACK_ROW_LENGTH, rowLength);
        pixelStorePack_.rowLength = rowLength;
    }
    #ifdef LLGL_OPENGL //TODO: emulate for GLES
    if (pixelStorePack_.imageHeight != imageHeight)
    {
        glPixelStorei(GL_PACK_IMAGE_HEIGHT, imageHeight);
        pixelStorePack_.imageHeight = imageHeight;
    }
    #endif
    if (pixelStorePack_.alignment != alignment)
    {
        glPixelStorei(GL_PACK_ALIGNMENT, alignment);
        pixelStorePack_.alignment = alignment;
    }
}

void GLStateManager::SetPixelStoreUnpack(GLint rowLength, GLint imageHeight, GLint alignment)
{
    if (pixelStoreUnpack_.rowLength != rowLength)
    {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
        pixelStoreUnpack_.rowLength = rowLength;
    }
    if (pixelStoreUnpack_.imageHeight != imageHeight)
    {
        glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, imageHeight);
        pixelStoreUnpack_.imageHeight = imageHeight;
    }
    if (pixelStoreUnpack_.alignment != alignment)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
        pixelStoreUnpack_.alignment = alignment;
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
    if (commonState_.depthFunc != func)
    {
        commonState_.depthFunc = func;
        glDepthFunc(func);
    }
}

void GLStateManager::SetDepthMask(GLboolean flag)
{
    if (commonState_.depthMask != flag)
    {
        commonState_.depthMask = flag;
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
        boundRasterizerState_ = nullptr;
}

void GLStateManager::BindRasterizerState(GLRasterizerState* rasterizerState)
{
    if (rasterizerState != nullptr && rasterizerState != boundRasterizerState_)
    {
        rasterizerState->Bind(*this);
        boundRasterizerState_ = rasterizerState;
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

void GLStateManager::SetBlendColor(const GLfloat* color)
{
    if ( color[0] != commonState_.blendColor[0] ||
         color[1] != commonState_.blendColor[1] ||
         color[2] != commonState_.blendColor[2] ||
         color[3] != commonState_.blendColor[3] )
    {
        commonState_.blendColor[0] = color[0];
        commonState_.blendColor[1] = color[1];
        commonState_.blendColor[2] = color[2];
        commonState_.blendColor[3] = color[3];
        glBlendColor(color[0], color[1], color[2], color[3]);
    }
}

void GLStateManager::SetLogicOp(GLenum opcode)
{
    #ifdef LLGL_OPENGL
    if (commonState_.logicOpCode != opcode)
    {
        commonState_.logicOpCode = opcode;
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
    if (bufferState_.boundBuffers[targetIdx] != buffer)
    {
        glBindBuffer(g_bufferTargetsEnum[targetIdx], buffer);
        bufferState_.boundBuffers[targetIdx] = buffer;
    }
}

void GLStateManager::BindBufferBase(GLBufferTarget target, GLuint index, GLuint buffer)
{
    /* Always bind buffer with a base index */
    auto targetIdx = static_cast<std::size_t>(target);
    glBindBufferBase(g_bufferTargetsEnum[targetIdx], index, buffer);
    bufferState_.boundBuffers[targetIdx] = buffer;
}

void GLStateManager::BindBuffersBase(GLBufferTarget target, GLuint first, GLsizei count, const GLuint* buffers)
{
    /* Always bind buffers with a base index */
    auto targetIdx = static_cast<std::size_t>(target);
    auto targetGL = g_bufferTargetsEnum[targetIdx];

    #ifdef GL_ARB_multi_bind
    if (HasExtension(GLExt::ARB_multi_bind))
    {
        /*
        Bind buffer array, but don't reset the currently bound buffer.
        The spec. of GL_ARB_multi_bind says, that the generic binding point is not modified by this function!
        */
        glBindBuffersBase(targetGL, first, count, buffers);
    }
    else
    #endif
    if (count > 0)
    {
        /* Bind each individual buffer, and store last bound buffer */
        bufferState_.boundBuffers[targetIdx] = buffers[count - 1];

        for (GLsizei i = 0; i < count; ++i)
            glBindBufferBase(targetGL, first + i, buffers[i]);
    }
}

void GLStateManager::BindBufferRange(GLBufferTarget target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    /* Always bind buffer with a base index */
    auto targetIdx = static_cast<std::size_t>(target);
    glBindBufferRange(g_bufferTargetsEnum[targetIdx], index, buffer, offset, size);
    bufferState_.boundBuffers[targetIdx] = buffer;
}

void GLStateManager::BindBuffersRange(GLBufferTarget target, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizeiptr* sizes)
{
    /* Always bind buffers with a base index */
    auto targetIdx = static_cast<std::size_t>(target);
    auto targetGL = g_bufferTargetsEnum[targetIdx];

    #ifdef GL_ARB_multi_bind
    if (HasExtension(GLExt::ARB_multi_bind))
    {
        /*
        Bind buffer array, but don't reset the currently bound buffer.
        The spec. of GL_ARB_multi_bind says, that the generic binding point is not modified by this function!
        */
        glBindBuffersRange(targetGL, first, count, buffers, offsets, sizes);
    }
    else
    #endif // /GL_ARB_multi_bind
    if (count > 0)
    {
        /* Bind each individual buffer, and store last bound buffer */
        bufferState_.boundBuffers[targetIdx] = buffers[count - 1];

        #ifdef GL_NV_transform_feedback
        if (HasExtension(GLExt::NV_transform_feedback))
        {
            for (GLsizei i = 0; i < count; ++i)
                glBindBufferRangeNV(targetGL, first + i, buffers[i], offsets[i], sizes[i]);
        }
        else
        #endif // /GL_NV_transform_feedback
        {
            for (GLsizei i = 0; i < count; ++i)
                glBindBufferRange(targetGL, first + i, buffers[i], offsets[i], sizes[i]);
        }
    }
}

void GLStateManager::UnbindBuffersBase(GLBufferTarget target, GLuint first, GLsizei count)
{
    BindBuffersBase(GLBufferTarget::UNIFORM_BUFFER, first, count, g_nullResources);
}

// Returns the maximum index value for the specified index data type.
static GLuint GetPrimitiveRestartIndex(bool indexType16Bits)
{
    return (indexType16Bits ? 0xFFFF : 0xFFFFFFFF);
}

void GLStateManager::BindVertexArray(GLuint vertexArray)
{
    /* Only bind VAO if it has changed */
    if (vertexArrayState_.boundVertexArray != vertexArray)
    {
        /* Bind VAO */
        glBindVertexArray(vertexArray);
        vertexArrayState_.boundVertexArray = vertexArray;

        #ifdef LLGL_GL_ENABLE_OPENGL2X
        /* Only perform deferred binding of element array buffer if VAOs are supported */
        if (HasExtension(GLExt::ARB_vertex_array_object))
        #endif // /LLGL_GL_ENABLE_OPENGL2X
        {
            /*
            Always reset index buffer binding
            -> see https://www.opengl.org/wiki/Vertex_Specification#Index_buffers
            */
            bufferState_.boundBuffers[static_cast<std::size_t>(GLBufferTarget::ELEMENT_ARRAY_BUFFER)] = 0;

            /* Bind deferred index buffer */
            if (vertexArray != 0 && vertexArrayState_.boundElementArrayBuffer != 0)
            {
                BindBuffer(
                    GLBufferTarget::ELEMENT_ARRAY_BUFFER,
                    vertexArrayState_.boundElementArrayBuffer
                );
                #ifdef LLGL_PRIMITIVE_RESTART
                SetPrimitiveRestartIndex(GetPrimitiveRestartIndex(vertexArrayState_.indexType16Bits));
                #endif
            }
        }
    }
}

void GLStateManager::BindGLBuffer(const GLBuffer& buffer)
{
    BindBuffer(buffer.GetTarget(), buffer.GetID());
}

void GLStateManager::NotifyVertexArrayRelease(GLuint vertexArray)
{
    InvalidateBoundGLObject(vertexArrayState_.boundVertexArray, vertexArray);
}

void GLStateManager::BindElementArrayBufferToVAO(GLuint buffer, bool indexType16Bits)
{
    #ifdef LLGL_GL_ENABLE_OPENGL2X
    if (!HasExtension(GLExt::ARB_vertex_array_object))
    {
        /* Bind element array buffer directly (for GL 2.x compatibility) */
        BindBuffer(GLBufferTarget::ELEMENT_ARRAY_BUFFER, buffer);
    }
    else
    #endif // /LLGL_GL_ENABLE_OPENGL2X
    {
        /* Always store buffer ID to bind the index buffer the next time "BindVertexArray" is called */
        vertexArrayState_.boundElementArrayBuffer   = buffer;
        vertexArrayState_.indexType16Bits           = indexType16Bits;

        /* If a valid VAO is currently being bound, bind the specified buffer directly */
        if (vertexArrayState_.boundVertexArray != 0)
        {
            BindBuffer(GLBufferTarget::ELEMENT_ARRAY_BUFFER, buffer);
            #ifdef LLGL_PRIMITIVE_RESTART
            SetPrimitiveRestartIndex(GetPrimitiveRestartIndex(vertexArrayState_.indexType16Bits));
            #endif
        }
    }
}

void GLStateManager::PushBoundBuffer(GLBufferTarget target)
{
    bufferState_.boundBufferStack.push(
        {
            target,
            bufferState_.boundBuffers[static_cast<std::size_t>(target)]
        }
    );
}

void GLStateManager::PopBoundBuffer()
{
    const auto& state = bufferState_.boundBufferStack.top();
    {
        BindBuffer(state.target, state.buffer);
    }
    bufferState_.boundBufferStack.pop();
}

void GLStateManager::NotifyBufferRelease(GLuint buffer, GLBufferTarget target)
{
    auto targetIdx = static_cast<std::size_t>(target);
    InvalidateBoundGLObject(bufferState_.boundBuffers[targetIdx], buffer);
}

void GLStateManager::NotifyBufferRelease(const GLBuffer& buffer)
{
    auto id         = buffer.GetID();
    auto bindFlags  = buffer.GetBindFlags();

    /* Release buffer ID from all potentially used GL buffer targets */
    if ((bindFlags & BindFlags::VertexBuffer) != 0)
        NotifyBufferRelease(id, GLBufferTarget::ARRAY_BUFFER);
    if ((bindFlags & BindFlags::IndexBuffer) != 0)
        NotifyBufferRelease(id, GLBufferTarget::ELEMENT_ARRAY_BUFFER);
    if ((bindFlags & BindFlags::ConstantBuffer) != 0)
        NotifyBufferRelease(id, GLBufferTarget::UNIFORM_BUFFER);
    if ((bindFlags & BindFlags::StreamOutputBuffer) != 0)
        NotifyBufferRelease(id, GLBufferTarget::TRANSFORM_FEEDBACK_BUFFER);
    if ((bindFlags & (BindFlags::Sampled | BindFlags::Storage)) != 0)
        NotifyBufferRelease(id, GLBufferTarget::SHADER_STORAGE_BUFFER);
    if ((bindFlags & BindFlags::IndirectBuffer) != 0)
    {
        NotifyBufferRelease(id, GLBufferTarget::DRAW_INDIRECT_BUFFER);
        NotifyBufferRelease(id, GLBufferTarget::DISPATCH_INDIRECT_BUFFER);
    }

    NotifyBufferRelease(id, GLBufferTarget::COPY_READ_BUFFER);
    NotifyBufferRelease(id, GLBufferTarget::COPY_WRITE_BUFFER);
}

void GLStateManager::DisableVertexAttribArrays(GLuint firstIndex)
{
    /* Disable remaining vertex-attrib-arrays */
    for (GLuint i = firstIndex; i < bufferState_.lastVertexAttribArray; ++i)
        glDisableVertexAttribArray(i);

    /* Store new highest vertex-attrib-array index */
    bufferState_.lastVertexAttribArray = firstIndex;
}

/* ----- Framebuffer ----- */

void GLStateManager::BindGLRenderTarget(GLRenderTarget* renderTarget)
{
    framebufferState_.boundRenderTarget = renderTarget;
    if (renderTarget)
        BindFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER, renderTarget->GetFramebuffer().GetID());
    else
        BindFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER, 0);
}

void GLStateManager::BindFramebuffer(GLFramebufferTarget target, GLuint framebuffer)
{
    /* Only bind framebuffer if the framebuffer has changed */
    auto targetIdx = static_cast<std::size_t>(target);
    if (framebufferState_.boundFramebuffers[targetIdx] != framebuffer)
    {
        framebufferState_.boundFramebuffers[targetIdx] = framebuffer;
        glBindFramebuffer(g_framebufferTargetsEnum[targetIdx], framebuffer);
    }
}

void GLStateManager::PushBoundFramebuffer(GLFramebufferTarget target)
{
    framebufferState_.boundFramebufferStack.push(
        {
            target,
            framebufferState_.boundFramebuffers[static_cast<std::size_t>(target)]
        }
    );
}

void GLStateManager::PopBoundFramebuffer()
{
    const auto& state = framebufferState_.boundFramebufferStack.top();
    {
        BindFramebuffer(state.target, state.framebuffer);
    }
    framebufferState_.boundFramebufferStack.pop();
}

void GLStateManager::NotifyFramebufferRelease(GLuint framebuffer)
{
    for (auto& boundFramebuffer : framebufferState_.boundFramebuffers)
        InvalidateBoundGLObject(boundFramebuffer, framebuffer);
}

void GLStateManager::NotifyGLRenderTargetRelease(GLRenderTarget* renderTarget)
{
    if (framebufferState_.boundRenderTarget == renderTarget)
        framebufferState_.boundRenderTarget = nullptr;
}

GLRenderTarget* GLStateManager::GetBoundRenderTarget() const
{
    return framebufferState_.boundRenderTarget;
}

/* ----- Renderbuffer ----- */

void GLStateManager::BindRenderbuffer(GLuint renderbuffer)
{
    if (renderbufferState_.boundRenderbuffer != renderbuffer)
    {
        renderbufferState_.boundRenderbuffer = renderbuffer;
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    }
}

void GLStateManager::PushBoundRenderbuffer()
{
    renderbufferState_.boundRenderbufferStack.push(renderbufferState_.boundRenderbuffer);
}

void GLStateManager::PopBoundRenderbuffer()
{
    const auto& state = renderbufferState_.boundRenderbufferStack.top();
    {
        BindRenderbuffer(state);
    }
    renderbufferState_.boundRenderbufferStack.pop();
}

void GLStateManager::DeleteRenderbuffer(GLuint renderbuffer)
{
    if (renderbuffer != 0)
    {
        glDeleteRenderbuffers(1, &renderbuffer);
        InvalidateBoundGLObject(renderbufferState_.boundRenderbuffer, renderbuffer);
    }
}

/* ----- Texture ----- */

GLTextureTarget GLStateManager::GetTextureTarget(const TextureType type)
{
    switch (type)
    {
        case TextureType::Texture1D:        return GLTextureTarget::TEXTURE_1D;
        case TextureType::Texture2D:        return GLTextureTarget::TEXTURE_2D;
        case TextureType::Texture3D:        return GLTextureTarget::TEXTURE_3D;
        case TextureType::TextureCube:      return GLTextureTarget::TEXTURE_CUBE_MAP;
        case TextureType::Texture1DArray:   return GLTextureTarget::TEXTURE_1D_ARRAY;
        case TextureType::Texture2DArray:   return GLTextureTarget::TEXTURE_2D_ARRAY;
        case TextureType::TextureCubeArray: return GLTextureTarget::TEXTURE_CUBE_MAP_ARRAY;
        case TextureType::Texture2DMS:      return GLTextureTarget::TEXTURE_2D_MULTISAMPLE;
        case TextureType::Texture2DMSArray: return GLTextureTarget::TEXTURE_2D_MULTISAMPLE_ARRAY;
        default:                            break;
    }
    throw std::invalid_argument("failed to convert texture type to OpenGL texture target");
}

void GLStateManager::ActiveTexture(std::uint32_t layer)
{
    #ifdef LLGL_DEBUG
    LLGL_ASSERT_UPPER_BOUND(layer, numTextureLayers);
    #endif

    if (textureState_.activeTexture != layer)
    {
        /* Active specified texture layer and store reference to bound textures array */
        SetActiveTextureLayer(layer);
        glActiveTexture(g_textureLayersEnum[layer]);
    }
}

void GLStateManager::BindTexture(GLTextureTarget target, GLuint texture)
{
    /* Only bind texutre if the texture has changed */
    auto targetIdx = static_cast<std::size_t>(target);
    if (textureState_.activeLayerRef->boundTextures[targetIdx] != texture)
    {
        textureState_.activeLayerRef->boundTextures[targetIdx] = texture;
        glBindTexture(g_textureTargetsEnum[targetIdx], texture);
    }
}

void GLStateManager::BindTextures(GLuint first, GLsizei count, const GLTextureTarget* targets, const GLuint* textures)
{
    #ifdef GL_ARB_multi_bind
    if (HasExtension(GLExt::ARB_multi_bind))
    {
        /* Store bound textures */
        for (GLsizei i = 0; i < count; ++i)
        {
            auto targetIdx = static_cast<std::size_t>(targets[i]);
            textureState_.layers[i + first].boundTextures[targetIdx] = textures[i];
        }

        /*
        Bind all textures at once, but don't reset the currently active texture layer.
        The spec. of GL_ARB_multi_bind states that the active texture slot is not modified by this function.
        see https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_multi_bind.txt
        */
        glBindTextures(first, count, textures);
    }
    else
    #endif
    {
        /* Bind each texture layer individually */
        for (GLsizei i = 0; i < count; ++i)
        {
            ActiveTexture(first + i);
            BindTexture(targets[i], textures[i]);
        }
    }
}

void GLStateManager::UnbindTextures(GLuint first, GLsizei count)
{
    #ifdef GL_ARB_multi_bind
    if (HasExtension(GLExt::ARB_multi_bind))
    {
        /* Reset bound textures */
        for (GLsizei i = 0; i < count; ++i)
        {
            auto& boundTextures = textureState_.layers[i].boundTextures;
            std::fill(std::begin(boundTextures), std::end(boundTextures), 0);
        }

        /*
        Unbind all textures at once, but don't reset the currently active texture layer.
        The spec. of GL_ARB_multi_bind states that the active texture slot is not modified by this function.
        see https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_multi_bind.txt
        */
        glBindTextures(first, count, nullptr);
    }
    else
    #endif // /GL_ARB_multi_bind
    {
        /* Unbind all targets for each texture layer individually */
        for (GLsizei i = 0; i < count; ++i)
        {
            ActiveTexture(first + i);
            for (std::size_t target = 0; target < numTextureTargets; ++target)
                BindTexture(static_cast<GLTextureTarget>(target), 0);
        }
    }
}

void GLStateManager::BindImageTexture(GLuint unit, GLint level, GLenum format, GLuint texture)
{
    #ifdef GL_ARB_shader_image_load_store
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
    #endif // /GL_ARB_shader_image_load_store
    {
        /* Error: extension not supported */
        throw std::runtime_error("renderer does not support storage images");
    }
}

void GLStateManager::BindImageTextures(GLuint first, GLsizei count, const GLenum* formats, const GLuint* textures)
{
    #ifdef GL_ARB_multi_bind
    if (HasExtension(GLExt::ARB_multi_bind))
    {
        /* Bind all image units at once */
        glBindImageTextures(first, count, textures);
    }
    else
    #endif // /GL_ARB_multi_bind
    {
        /* Bind image units individually */
        for (GLsizei i = 0; i < count; ++i)
            BindImageTexture(first + static_cast<GLuint>(i), 0, formats[i], textures[i]);
    }
}

void GLStateManager::UnbindImageTextures(GLuint first, GLsizei count)
{
    #ifdef GL_ARB_multi_bind
    if (HasExtension(GLExt::ARB_multi_bind))
    {
        /* Bind all image units at once */
        glBindImageTextures(first, count, nullptr);
    }
    else
    #endif // /GL_ARB_multi_bind
    {
        /* Unbind all image units individually */
        for (GLsizei i = 0; i < count; ++i)
            BindImageTexture(first + static_cast<GLuint>(i), 0, 0, 0);
    }
}

void GLStateManager::PushBoundTexture(std::uint32_t layer, GLTextureTarget target)
{
    #ifdef LLGL_DEBUG
    LLGL_ASSERT_UPPER_BOUND(layer, numTextureLayers);
    #endif

    textureState_.boundTextureStack.push(
        {
            layer,
            target,
            (textureState_.layers[layer].boundTextures[static_cast<std::size_t>(target)])
        }
    );
}

void GLStateManager::PushBoundTexture(GLTextureTarget target)
{
    PushBoundTexture(textureState_.activeTexture, target);
}

void GLStateManager::PopBoundTexture()
{
    const auto& state = textureState_.boundTextureStack.top();
    {
        ActiveTexture(state.layer);
        BindTexture(state.target, state.texture);
    }
    textureState_.boundTextureStack.pop();
}

void GLStateManager::BindGLTexture(const GLTexture& texture)
{
    BindTexture(GLStateManager::GetTextureTarget(texture.GetType()), texture.GetID());
}

void GLStateManager::DeleteTexture(GLuint texture, GLTextureTarget target, bool activeLayerOnly)
{
    if (texture != 0)
    {
        glDeleteTextures(1, &texture);
        NotifyTextureRelease(texture, target, activeLayerOnly);
    }
}

/* ----- Sampler ----- */

void GLStateManager::BindSampler(GLuint layer, GLuint sampler)
{
    #ifdef LLGL_DEBUG
    LLGL_ASSERT_UPPER_BOUND(layer, numTextureLayers);
    #endif

    if (samplerState_.boundSamplers[layer] != sampler)
    {
        samplerState_.boundSamplers[layer] = sampler;
        glBindSampler(layer, sampler);
    }
}

void GLStateManager::BindSamplers(GLuint first, GLsizei count, const GLuint* samplers)
{
    #ifdef GL_ARB_multi_bind
    if (count >= 2 && HasExtension(GLExt::ARB_multi_bind))
    {
        /* Store bound samplers */
        for (GLsizei i = 0; i < count; ++i)
            samplerState_.boundSamplers[i + first] = samplers[i];

        /* Bind all samplers at once */
        glBindSamplers(first, count, samplers);
    }
    else
    #endif
    {
        /* Bind each sampler individually */
        for (GLsizei i = 0; i < count; ++i)
            BindSampler(first + static_cast<GLuint>(i), samplers[i]);
    }
}

void GLStateManager::UnbindSamplers(GLuint first, GLsizei count)
{
    BindSamplers(first, count, g_nullResources);
}

void GLStateManager::NotifySamplerRelease(GLuint sampler)
{
    for (auto& boundSampler : samplerState_.boundSamplers)
        InvalidateBoundGLObject(boundSampler, sampler);
}

/* ----- Shader binding ----- */

void GLStateManager::BindShaderProgram(GLuint program)
{
    if (shaderState_.boundProgram != program)
    {
        shaderState_.boundProgram = program;
        glUseProgram(program);
    }
}

void GLStateManager::NotifyShaderProgramRelease(GLuint program)
{
    InvalidateBoundGLObject(shaderState_.boundProgram, program);
}

GLuint GLStateManager::GetBoundShaderProgram() const
{
    return shaderState_.boundProgram;
}

/* ----- Render pass ----- */

void GLStateManager::BindRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    const GLClearValue& defaultClearValue)
{
    /* Bind render target/context */
    if (renderTarget.IsRenderContext())
        BindAndBlitRenderContext(LLGL_CAST(GLRenderContext&, renderTarget));
    else
        BindAndBlitRenderTarget(LLGL_CAST(GLRenderTarget&, renderTarget));

    /* Clear attachments */
    if (renderPass)
    {
        auto renderPassGL = LLGL_CAST(const GLRenderPass*, renderPass);
        ClearAttachmentsWithRenderPass(*renderPassGL, numClearValues, clearValues, defaultClearValue);
    }
}

void GLStateManager::Clear(long flags)
{
    /* Setup GL clear mask and clear respective buffer */
    GLbitfield mask = 0;

    if ((flags & ClearFlags::Color) != 0)
    {
        PushColorMaskAndEnable();
        mask |= GL_COLOR_BUFFER_BIT;
    }

    if ((flags & ClearFlags::Depth) != 0)
    {
        PushDepthMaskAndEnable();
        mask |= GL_DEPTH_BUFFER_BIT;
    }

    if ((flags & ClearFlags::Stencil) != 0)
    {
        #if 0//TODO
        stateMngr_->SetStencilMask(GL_TRUE);
        #endif
        mask |= GL_STENCIL_BUFFER_BIT;
    }

    /* Clear buffers */
    glClear(mask);

    /* Restore framebuffer masks */
    #if 0//TODO
    if ((flags & ClearFlags::Stencil) != 0)
        stateMngr_->PopStencilMask();
    #endif
    if ((flags & ClearFlags::Depth) != 0)
        PopDepthMask();
    if ((flags & ClearFlags::Color) != 0)
        PopColorMask();
}

void GLStateManager::ClearBuffers(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    bool clearedDepth = false, clearedColor = false;

    for (; numAttachments-- > 0; ++attachments)
    {
        if ((attachments->flags & ClearFlags::Color) != 0)
        {
            /* Enable color mask temporarily */
            PushColorMaskAndEnable();
            clearedColor = true;

            /* Clear color buffer */
            glClearBufferfv(
                GL_COLOR,
                static_cast<GLint>(attachments->colorAttachment),
                attachments->clearValue.color.Ptr()
            );
        }
        else if ((attachments->flags & ClearFlags::DepthStencil) == ClearFlags::DepthStencil)
        {
            /* Enable depth mask temporarily */
            PushDepthMaskAndEnable();
            clearedDepth = true;

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
            /* Enable depth mask temporarily */
            PushDepthMaskAndEnable();
            clearedDepth = true;

            /* Clear only depth buffer */
            glClearBufferfv(GL_DEPTH, 0, &(attachments->clearValue.depth));
        }
        else if ((attachments->flags & ClearFlags::Stencil) != 0)
        {
            /* Clear only stencil buffer */
            GLint stencil = static_cast<GLint>(attachments->clearValue.stencil);
            glClearBufferiv(GL_STENCIL, 0, &stencil);
        }
    }

    if (clearedDepth)
        PopDepthMask();
    if (clearedColor)
        PopColorMask();
}


/*
 * ======= Private: =======
 */

void GLStateManager::AssertExtViewportArray()
{
    #ifdef GL_ARB_viewport_array
    if (!HasExtension(GLExt::ARB_viewport_array))
    #endif
    {
        throw std::runtime_error("renderer does not support viewport, depth-range, and scissor arrays");
    }
}

void GLStateManager::SetActiveTextureLayer(std::uint32_t layer)
{
    textureState_.activeTexture     = layer;
    textureState_.activeLayerRef    = &(textureState_.layers[textureState_.activeTexture]);
}

void GLStateManager::NotifyTextureRelease(GLuint texture, GLTextureTarget target, bool activeLayerOnly)
{
    auto targetIdx = static_cast<std::size_t>(target);
    if (activeLayerOnly)
    {
        /* Invalidate GL texture only on active layer (should only be used for internal and temporary textures) */
        InvalidateBoundGLObject(textureState_.activeLayerRef->boundTextures[targetIdx], texture);
    }
    else
    {
        /* Invalidate GL texture on all layers */
        for (auto& layer : textureState_.layers)
            InvalidateBoundGLObject(layer.boundTextures[targetIdx], texture);
    }
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
    #ifdef GL_KHR_debug
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
    limits_.maxTextureLayers = std::min(GLStateManager::numTextureLayers, static_cast<GLuint>(maxTextureImageUnits));

    /* Get maximum number of image units */
    #ifdef GL_ARB_shader_image_load_store
    if (HasExtension(GLExt::ARB_shader_image_load_store))
    {
        GLint maxImageUnits = 0;
        glGetIntegerv(GL_MAX_IMAGE_UNITS, &maxImageUnits);
        limits_.maxImageUnits = std::min(GLStateManager::numImageUnits, static_cast<GLuint>(maxImageUnits));
    }
    #endif // /GL_ARB_shader_image_load_store

    /* Accumulate common limitations */
    AccumCommonGLLimits(GLStateManager::commonLimits_, limits_);
}

#ifdef LLGL_GL_ENABLE_VENDOR_EXT

void GLStateManager::DetermineVendorSpecificExtensions()
{
    #if defined GL_NV_conservative_raster || defined GL_INTEL_conservative_rasterization

    /* Initialize extenstion states */
    auto InitStateExt = [&](GLStateExt state, const GLExt extension, GLenum cap)
    {
        auto idx = static_cast<std::size_t>(state);
        auto& val = capabilityStateExt_.values[idx];
        if (val.cap == 0 && HasExtension(extension))
        {
            val.cap     = cap;
            val.enabled = (glIsEnabled(cap) != GL_FALSE);
        }
    };

    #ifdef GL_NV_conservative_raster
    // see https://www.opengl.org/registry/specs/NV/conservative_raster.txt
    InitStateExt(GLStateExt::CONSERVATIVE_RASTERIZATION, GLExt::NV_conservative_raster, GL_CONSERVATIVE_RASTERIZATION_NV);
    #endif

    #ifdef GL_INTEL_conservative_rasterization
    // see https://www.opengl.org/registry/specs/INTEL/conservative_rasterization.txt
    InitStateExt(GLStateExt::CONSERVATIVE_RASTERIZATION, GLExt::INTEL_conservative_rasterization, GL_CONSERVATIVE_RASTERIZATION_INTEL);
    #endif

    #endif
}

#endif

/* ----- Stacks ----- */

void GLStateManager::PushDepthMaskAndEnable()
{
    /* Cache current depth mask, then enable it */
    commonState_.cachedDepthMask = commonState_.depthMask;
    SetDepthMask(GL_TRUE);
}

void GLStateManager::PopDepthMask()
{
    /* Restore cached depth mask */
    SetDepthMask(commonState_.cachedDepthMask);
}

void GLStateManager::PushColorMaskAndEnable()
{
    if (!colorMaskOnStack_)
    {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        colorMaskOnStack_ = true;
    }
}

void GLStateManager::PopColorMask()
{
    if (colorMaskOnStack_)
    {
        if (boundBlendState_)
            boundBlendState_->BindColorMaskOnly(*this);
        colorMaskOnStack_ = false;
    }
}

/* ----- Render pass ----- */

void GLStateManager::BlitBoundRenderTarget()
{
    if (auto renderTarget = GetBoundRenderTarget())
        renderTarget->BlitOntoFramebuffer();
}

void GLStateManager::BindAndBlitRenderTarget(GLRenderTarget& renderTargetGL)
{
    /* Blit previously bound render target, bind FBO, and notify new render target height */
    BlitBoundRenderTarget();
    BindGLRenderTarget(&renderTargetGL);
    NotifyRenderTargetHeight(static_cast<GLint>(renderTargetGL.GetResolution().height));
}

void GLStateManager::BindAndBlitRenderContext(GLRenderContext& renderContextGL)
{
    /* Blit previously bound render target, unbind FBO, and make context current */
    BlitBoundRenderTarget();
    BindGLRenderTarget(nullptr);
    GLRenderContext::GLMakeCurrent(&renderContextGL);
}

void GLStateManager::ClearAttachmentsWithRenderPass(
    const GLRenderPass& renderPassGL,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    const GLClearValue& defaultClearValue)
{
    auto mask = renderPassGL.GetClearMask();

    /* Clear color attachments */
    std::uint32_t idx = 0;
    if ((mask & GL_COLOR_BUFFER_BIT) != 0)
    {
        if (ClearColorBuffers(renderPassGL.GetClearColorAttachments(), numClearValues, clearValues, idx, defaultClearValue) > 0)
            PopColorMask();
    }

    /* Clear depth-stencil attachment */
    switch (mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
    {
        case (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT):
        {
            PushDepthMaskAndEnable();
            {
                /* Clear depth and stencil buffer simultaneously */
                if (idx < numClearValues)
                    glClearBufferfi(GL_DEPTH_STENCIL, 0, clearValues[idx].depth, static_cast<GLint>(clearValues[idx].stencil));
                else
                    glClearBufferfi(GL_DEPTH_STENCIL, 0, defaultClearValue.depth, defaultClearValue.stencil);
            }
            PopDepthMask();
        }
        break;

        case GL_DEPTH_BUFFER_BIT:
        {
            PushDepthMaskAndEnable();
            {
                /* Clear only depth buffer */
                if (idx < numClearValues)
                    glClearBufferfv(GL_DEPTH, 0, &(clearValues[idx].depth));
                else
                    glClearBufferfv(GL_DEPTH, 0, &(defaultClearValue.depth));
            }
            PopDepthMask();
        }
        break;

        case GL_STENCIL_BUFFER_BIT:
        {
            /* Clear only stencil buffer */
            if (idx < numClearValues)
            {
                GLint stencil = static_cast<GLint>(clearValues[idx].stencil);
                glClearBufferiv(GL_STENCIL, 0, &stencil);
            }
            else
                glClearBufferiv(GL_STENCIL, 0, &(defaultClearValue.stencil));
        }
        break;
    }
}

std::uint32_t GLStateManager::ClearColorBuffers(
    const std::uint8_t* colorBuffers,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    std::uint32_t&      idx,
    const GLClearValue& defaultClearValue)
{
    std::uint32_t i = 0, n = 0;

    /* Use specified clear values */
    for (; i < numClearValues; ++i)
    {
        /* Check if attachment list has ended */
        if (colorBuffers[i] != 0xFF)
        {
            PushColorMaskAndEnable();
            glClearBufferfv(GL_COLOR, static_cast<GLint>(colorBuffers[i]), clearValues[idx++].color.Ptr());
            ++n;
        }
        else
            return n;
    }

    /* Use default clear values */
    for (; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS; ++i)
    {
        /* Check if attachment list has ended */
        if (colorBuffers[i] != 0xFF)
        {
            PushColorMaskAndEnable();
            glClearBufferfv(GL_COLOR, static_cast<GLint>(colorBuffers[i]), defaultClearValue.color);
            ++n;
        }
        else
            return n;
    }

    return n;
}


} // /namespace LLGL



// ================================================================================
