/*
 * GLStateManager.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLStateManager.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionLoader.h"
#include "../../Assertion.h"
#include "../../../Core/Helper.h"
#include "../../GLCommon/GLTypes.h"


namespace LLGL
{


/* ----- Internal constants ---- */

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

static const GLenum framebufferTargetsMap[] =
{
    GL_FRAMEBUFFER,
    GL_DRAW_FRAMEBUFFER,
    GL_READ_FRAMEBUFFER,
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

static const GLenum textureLayersMap[] = 
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


/* ----- Common ----- */

GLStateManager* GLStateManager::active = nullptr;

GLStateManager::GLStateManager()
{
    /* Initialize all states with zero */
    Fill(renderState_.values, false);
    Fill(bufferState_.boundBuffers, 0);
    Fill(frameBufferState_.boundFrameBuffers, 0);
    Fill(samplerState_.boundSamplers, 0);

    for (auto& layer : textureState_.layers)
        Fill(layer.boundTextures, 0);

    SetActiveTextureLayer(0);

    /* Make this to the active state manager */
    GLStateManager::active = this;
}

void GLStateManager::DetermineExtensions()
{
    #ifdef LLGL_GL_ENABLE_VENDOR_EXT

    /* Initialize extenstion states */
    auto InitStateExt = [&](GLStateExt state, const GLExt extension, GLenum cap)
    {
        auto idx = static_cast<std::size_t>(state);
        auto& val = renderStateExt_.values[idx];
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

void GLStateManager::NotifyRenderTargetHeight(GLint height)
{
    /* Store new render-target height */
    renderTargetHeight_ = height;

    /* Update viewports */
    //todo...
}

void GLStateManager::SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state)
{
    /* Check for necessary updates */
    bool updateFrontFace = (gfxDependentState_.stateOpenGL.invertFrontFace != state.stateOpenGL.invertFrontFace);

    /* Store new graphics state */
    gfxDependentState_ = state;

    /* Update front face */
    if (updateFrontFace)
        SetFrontFace(commonState_.frontFaceAct);

    /* Set logical operation */
    if (state.stateOpenGL.logicOp != LogicOp::Keep)
    {
        if (state.stateOpenGL.logicOp != LogicOp::Disabled)
        {
            Enable(GLState::COLOR_LOGIC_OP);
            SetLogicOp(GLTypes::Map(state.stateOpenGL.logicOp));
        }
        else
            Disable(GLState::COLOR_LOGIC_OP);
    }

    /* Set line width */
    if (state.stateOpenGL.lineWidth > 0.0f)
        glLineWidth(state.stateOpenGL.lineWidth);
}

/* ----- Boolean states ----- */

void GLStateManager::Reset()
{
    /* Query all states from OpenGL */
    for (std::size_t i = 0; i < numStates; ++i)
        renderState_.values[i] = (glIsEnabled(stateCapsMap[i]) != GL_FALSE);
}

void GLStateManager::Set(GLState state, bool value)
{
    auto idx = static_cast<std::size_t>(state);
    if (renderState_.values[idx] != value)
    {
        renderState_.values[idx] = value;
        if (value)
            glEnable(stateCapsMap[idx]);
        else
            glDisable(stateCapsMap[idx]);
    }
}

void GLStateManager::Enable(GLState state)
{
    auto idx = static_cast<std::size_t>(state);
    if (!renderState_.values[idx])
    {
        renderState_.values[idx] = true;
        glEnable(stateCapsMap[idx]);
    }
}

void GLStateManager::Disable(GLState state)
{
    auto idx = static_cast<std::size_t>(state);
    if (renderState_.values[idx])
    {
        renderState_.values[idx] = false;
        glDisable(stateCapsMap[idx]);
    }
}

bool GLStateManager::IsEnabled(GLState state) const
{
    auto idx = static_cast<std::size_t>(state);
    return renderState_.values[idx];
}

void GLStateManager::PushState(GLState state)
{
    renderState_.valueStack.push(
        {
            state,
            renderState_.values[static_cast<std::size_t>(state)]
        }
    );
}

void GLStateManager::PopState()
{
    const auto& state = renderState_.valueStack.top();
    {
        Set(state.state, state.enabled);
    }
    renderState_.valueStack.pop();
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
    auto& val = renderStateExt_.values[idx];
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
    auto& val = renderStateExt_.values[idx];
    if (val.cap != 0 && !val.enabled)
    {
        val.enabled = true;
        glEnable(val.cap);
    }
}

void GLStateManager::Disable(GLStateExt state)
{
    auto idx = static_cast<std::size_t>(state);
    auto& val = renderStateExt_.values[idx];
    if (val.cap != 0 && val.enabled)
    {
        val.enabled = false;
        glDisable(val.cap);
    }
}

bool GLStateManager::IsEnabled(GLStateExt state) const
{
    auto idx = static_cast<std::size_t>(state);
    return renderStateExt_.values[idx].enabled;
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
    if (emulateClipControl_ && !gfxDependentState_.stateOpenGL.screenSpaceOriginLowerLeft)
        AdjustViewport(viewport);

    glViewport(
        static_cast<GLint>(viewport.x),
        static_cast<GLint>(viewport.y),
        static_cast<GLsizei>(viewport.width),
        static_cast<GLsizei>(viewport.height)
    );
}

void GLStateManager::SetViewportArray(std::vector<GLViewport>&& viewports)
{
    if (viewports.size() > 1)
    {
        AssertExtViewportArray();

        if (emulateClipControl_ && !gfxDependentState_.stateOpenGL.screenSpaceOriginLowerLeft)
        {
            for (auto& vp : viewports)
                AdjustViewport(vp);
        }

        glViewportArrayv(
            0,
            static_cast<GLsizei>(viewports.size()),
            reinterpret_cast<const GLfloat*>(viewports.data())
        );
    }
    else if (viewports.size() == 1)
        SetViewport(viewports.front());
}

void GLStateManager::SetDepthRange(GLDepthRange& depthRange)
{
    glDepthRange(depthRange.minDepth, depthRange.maxDepth);
}

void GLStateManager::SetDepthRangeArray(std::vector<GLDepthRange>&& depthRanges)
{
    if (depthRanges.size() > 1)
    {
        AssertExtViewportArray();

        glDepthRangeArrayv(
            0,
            static_cast<GLsizei>(depthRanges.size()),
            reinterpret_cast<const GLdouble*>(depthRanges.data())
        );
    }
    else if (depthRanges.size() == 1)
        SetDepthRange(depthRanges.front());
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

void GLStateManager::SetScissorArray(std::vector<GLScissor>&& scissors)
{
    if (scissors.size() > 1)
    {
        AssertExtViewportArray();

        if (emulateClipControl_)
        {
            for (auto& sc : scissors)
                AdjustScissor(sc);
        }

        glScissorArrayv(
            0,
            static_cast<GLsizei>(scissors.size()),
            reinterpret_cast<const GLint*>(scissors.data())
        );
    }
    else if (scissors.size() == 1)
        SetScissor(scissors.front());
}

void GLStateManager::SetBlendStates(const std::vector<GLBlend>& blendStates, bool blendEnabled)
{
    if (blendStates.size() == 1)
    {
        /* Set blend state only for the single draw buffer */
        const auto& state = blendStates.front();

        glColorMask(state.colorMask.r, state.colorMask.g, state.colorMask.b, state.colorMask.a);
        if (blendEnabled)
        {
            glBlendFuncSeparate(state.srcColor, state.destColor, state.srcAlpha, state.destAlpha);
            glBlendEquationSeparate(state.funcColor, state.funcAlpha);
        }
    }
    else if (blendStates.size() > 1)
    {
        GLenum drawBuffer = GL_COLOR_ATTACHMENT0;

        /* Set respective blend state for each draw buffer */
        for (const auto& state : blendStates)
            SetBlendState(drawBuffer++, state, blendEnabled);
    }
}

void GLStateManager::SetBlendState(GLuint drawBuffer, const GLBlend& state, bool blendEnabled)
{
    #ifndef __APPLE__
    if (HasExtension(GLExt::ARB_draw_buffers_blend))
    #endif
    {
        glColorMaski(drawBuffer, state.colorMask.r, state.colorMask.g, state.colorMask.b, state.colorMask.a);

        if (blendEnabled)
        {
            glBlendFuncSeparatei(drawBuffer, state.srcColor, state.destColor, state.srcAlpha, state.destAlpha);
            glBlendEquationSeparatei(drawBuffer, state.funcColor, state.funcAlpha);
        }
    }
    #ifndef __APPLE__
    else
    {
        glDrawBuffer(drawBuffer);
        glColorMask(state.colorMask.r, state.colorMask.g, state.colorMask.b, state.colorMask.a);

        if (blendEnabled)
        {
            glBlendFuncSeparate(state.srcColor, state.destColor, state.srcAlpha, state.destAlpha);
            glBlendEquationSeparate(state.funcColor, state.funcAlpha);
        }
    }
    #endif
}

void GLStateManager::SetClipControl(GLenum origin, GLenum depth)
{
    /*if (HasExtension(GLExt::ARB_clip_control))
        glClipControl(origin, depth);
    else*/
        emulateClipControl_ = (origin == GL_UPPER_LEFT);
}

void GLStateManager::SetDepthFunc(GLenum func)
{
    if (commonState_.depthFunc != func)
    {
        commonState_.depthFunc = func;
        glDepthFunc(func);
    }
}

void GLStateManager::SetStencilState(GLenum face, const GLStencil& state)
{
    switch (face)
    {
        case GL_FRONT:
            SetStencilState(GL_FRONT, commonState_.stencil[0], state);
            break;
        case GL_BACK:
            SetStencilState(GL_BACK, commonState_.stencil[1], state);
            break;
        case GL_FRONT_AND_BACK:
            SetStencilState(GL_FRONT, commonState_.stencil[0], state);
            SetStencilState(GL_BACK, commonState_.stencil[1], state);
            break;
    }
}

void GLStateManager::SetStencilState(GLenum face, GLStencil& to, const GLStencil& from)
{
    if (to.sfail != from.sfail || to.dpfail != from.dpfail || to.dppass != from.dppass)
    {
        to.sfail    = from.sfail;
        to.dpfail   = from.dpfail;
        to.dppass   = from.dppass;
        glStencilOpSeparate(face, to.sfail, to.dpfail, to.dppass);
    }

    if (to.func != from.func || to.ref != from.ref || to.mask != from.mask)
    {
        to.func = from.func;
        to.ref  = from.ref;
        to.mask = from.mask;
        glStencilFuncSeparate(face, to.func, to.ref, to.mask);
    }

    if (to.writeMask != from.writeMask)
    {
        to.writeMask = from.writeMask;
        glStencilMaskSeparate(face, to.writeMask);
    }
}

void GLStateManager::SetPolygonMode(GLenum mode)
{
    if (commonState_.polygonMode != mode)
    {
        commonState_.polygonMode = mode;
        glPolygonMode(GL_FRONT_AND_BACK, mode);
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
    if (gfxDependentState_.stateOpenGL.invertFrontFace)
        mode = (mode == GL_CW ? GL_CCW : GL_CW);

    /* Set front face */
    if (commonState_.frontFace != mode)
    {
        commonState_.frontFace = mode;
        glFrontFace(mode);
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

void GLStateManager::SetPatchVertices(GLint patchVertices)
{
    if (commonState_.patchVertices_ != patchVertices)
    {
        commonState_.patchVertices_ = patchVertices;
        glPatchParameteri(GL_PATCH_VERTICES, patchVertices);
    }
}

void GLStateManager::SetBlendColor(const ColorRGBAf& color)
{
    if (!Gs::Equals(color, commonState_.blendColor))
    {
        commonState_.blendColor = color;
        glBlendColor(color.r, color.g, color.b, color.a);
    }
}

void GLStateManager::SetLogicOp(GLenum opcode)
{
    if (commonState_.logicOpCode != opcode)
    {
        commonState_.logicOpCode = opcode;
        glLogicOp(opcode);
    }
}

/* ----- Buffer binding ----- */

void GLStateManager::BindBuffer(GLBufferTarget target, GLuint buffer)
{
    /* Only bind buffer if the buffer has changed */
    auto targetIdx = static_cast<std::size_t>(target);
    if (bufferState_.boundBuffers[targetIdx] != buffer)
    {
        glBindBuffer(bufferTargetsMap[targetIdx], buffer);
        bufferState_.boundBuffers[targetIdx] = buffer;
    }
}

void GLStateManager::BindBufferBase(GLBufferTarget target, GLuint index, GLuint buffer)
{
    /* Always bind buffer with a base index */
    auto targetIdx = static_cast<std::size_t>(target);
    glBindBufferBase(bufferTargetsMap[targetIdx], index, buffer);
    bufferState_.boundBuffers[targetIdx] = buffer;
}

void GLStateManager::BindBuffersBase(GLBufferTarget target, GLuint first, GLsizei count, const GLuint* buffers)
{
    /* Always bind buffers with a base index */
    auto targetIdx = static_cast<std::size_t>(target);
    auto targetGL = bufferTargetsMap[targetIdx];
    
    #ifndef __APPLE__
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

        while (count-- > 0)
        {
            glBindBufferBase(targetGL, first, *buffers);
            ++buffers;
            ++first;
        }
    }
}

void GLStateManager::BindVertexArray(GLuint vertexArray)
{
    /* Only bind VAO if it has changed */
    if (vertexArrayState_.boundVertexArray != vertexArray)
    {
        /* Bind VAO */
        glBindVertexArray(vertexArray);
        vertexArrayState_.boundVertexArray = vertexArray;

        /*
        Always reset index buffer binding
        -> see https://www.opengl.org/wiki/Vertex_Specification#Index_buffers
        */
        bufferState_.boundBuffers[static_cast<std::size_t>(GLBufferTarget::ELEMENT_ARRAY_BUFFER)] = 0;

        /* Bind deferred index buffer */
        if (vertexArray != 0 && vertexArrayState_.deferredBoundIndexBuffer != 0)
        {
            BindBuffer(
                GLBufferTarget::ELEMENT_ARRAY_BUFFER,
                vertexArrayState_.deferredBoundIndexBuffer
            );
        }
    }
}

void GLStateManager::DeferredBindIndexBuffer(GLuint buffer)
{
    /* Always store buffer ID to bind the index buffer the next time "BindVertexArray" is called */
    vertexArrayState_.deferredBoundIndexBuffer = buffer;

    /* If a valid VAO is currently being bound, bind the specified buffer directly */
    if (vertexArrayState_.boundVertexArray != 0)
        BindBuffer(GLBufferTarget::ELEMENT_ARRAY_BUFFER, buffer);
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

static GLBufferTarget GetGLBufferTarget(const BufferType type)
{
    switch (type)
    {
        case BufferType::Vertex:        return GLBufferTarget::ARRAY_BUFFER;
        case BufferType::Index:         return GLBufferTarget::ELEMENT_ARRAY_BUFFER;
        case BufferType::Constant:      return GLBufferTarget::UNIFORM_BUFFER;
        case BufferType::Storage:       return GLBufferTarget::SHADER_STORAGE_BUFFER;
        case BufferType::StreamOutput:  return GLBufferTarget::TRANSFORM_FEEDBACK_BUFFER;
    }
    throw std::invalid_argument("failed to map 'BufferType' to internal type 'GLBufferTarget'");
}

void GLStateManager::BindBuffer(const GLBuffer& buffer)
{
    BindBuffer(GetGLBufferTarget(buffer.GetType()), buffer.GetID());
}

/* ----- Framebuffer binding ----- */

void GLStateManager::BindFrameBuffer(GLFrameBufferTarget target, GLuint framebuffer)
{
    /* Only bind framebuffer if the framebuffer has changed */
    auto targetIdx = static_cast<std::size_t>(target);
    if (frameBufferState_.boundFrameBuffers[targetIdx] != framebuffer)
    {
        frameBufferState_.boundFrameBuffers[targetIdx] = framebuffer;
        glBindFramebuffer(framebufferTargetsMap[targetIdx], framebuffer);
    }
}

void GLStateManager::PushBoundFrameBuffer(GLFrameBufferTarget target)
{
    frameBufferState_.boundFrameBufferStack.push(
        {
            target,
            frameBufferState_.boundFrameBuffers[static_cast<std::size_t>(target)]
        }
    );
}

void GLStateManager::PopBoundFrameBuffer()
{
    const auto& state = frameBufferState_.boundFrameBufferStack.top();
    {
        BindFrameBuffer(state.target, state.buffer);
    }
    frameBufferState_.boundFrameBufferStack.pop();
}

/* ----- Renderbuffer binding ----- */

void GLStateManager::BindRenderBuffer(GLuint renderbuffer)
{
    if (renderBufferState_.boundRenderBuffer != renderbuffer)
    {
        renderBufferState_.boundRenderBuffer = renderbuffer;
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    }
}

void GLStateManager::PushBoundRenderBuffer()
{
    renderBufferState_.boundRenderBufferStack.push(renderBufferState_.boundRenderBuffer);
}

void GLStateManager::PopBoundRenderBuffer()
{
    BindRenderBuffer(renderBufferState_.boundRenderBufferStack.top());
    renderBufferState_.boundRenderBufferStack.pop();
}

/* ----- Texture binding ----- */

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

void GLStateManager::ActiveTexture(unsigned int layer)
{
    #ifdef LLGL_DEBUG
    LLGL_ASSERT_RANGE(layer, numTextureLayers);
    #endif

    if (textureState_.activeTexture != layer)
    {
        /* Active specified texture layer and store reference to bound textures array */
        SetActiveTextureLayer(layer);
        glActiveTexture(textureLayersMap[layer]);
    }
}

void GLStateManager::BindTexture(GLTextureTarget target, GLuint texture)
{
    /* Only bind texutre if the texture has changed */
    auto targetIdx = static_cast<std::size_t>(target);
    if (activeTextureLayer_->boundTextures[targetIdx] != texture)
    {
        activeTextureLayer_->boundTextures[targetIdx] = texture;
        glBindTexture(textureTargetsMap[targetIdx], texture);
    }
}

void GLStateManager::BindTextures(GLuint first, GLsizei count, const GLTextureTarget* targets, const GLuint* textures)
{
    #ifndef __APPLE__
    if (HasExtension(GLExt::ARB_multi_bind))
    {
        /* Store bound textures */
        for (GLsizei i = 0; i < count; ++i)
        {
            auto targetIdx = static_cast<std::size_t>(targets[i]);
            textureState_.layers[i].boundTextures[targetIdx] = textures[i];
        }

        /*
        Bind all textures at once, but don't reset the currently active texture layer.
        The spec. of GL_ARB_multi_bind says, that the active texture slot is not modified by this function!
        */
        glBindTextures(first, count, textures);
    }
    else
    #endif
    {
        /* Bind each texture layer individually */
        while (count-- > 0)
        {
            ActiveTexture(first);
            BindTexture(*targets, *textures);
            ++targets;
            ++textures;
            ++first;
        }
    }
}

void GLStateManager::PushBoundTexture(unsigned int layer, GLTextureTarget target)
{
    #ifdef LLGL_DEBUG
    LLGL_ASSERT_RANGE(layer, numTextureLayers);
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

void GLStateManager::BindTexture(const GLTexture& texture)
{
    BindTexture(GLStateManager::GetTextureTarget(texture.GetType()), texture.GetID());
}

/* ----- Sampler binding ----- */

void GLStateManager::BindSampler(unsigned int layer, GLuint sampler)
{
    #ifdef LLGL_DEBUG
    LLGL_ASSERT_RANGE(layer, numTextureLayers);
    #endif

    if (samplerState_.boundSamplers[layer] != sampler)
    {
        samplerState_.boundSamplers[layer] = sampler;
        glBindSampler(layer, sampler);
    }
}

void GLStateManager::BindSamplers(unsigned int first, unsigned int count, const GLuint* samplers)
{
    #ifndef __APPLE__
    if (HasExtension(GLExt::ARB_multi_bind))
    {
        /* Bind all samplers at once */
        glBindSamplers(first, static_cast<GLsizei>(count), samplers);

        /* Store bound textures */
        for (unsigned int i = 0; i < count; ++i)
            samplerState_.boundSamplers[i] = samplers[i];
    }
    else
    #endif
    {
        /* Bind each sampler individually */
        for (unsigned int i = 0; i < count; ++i)
            BindSampler(first + i, samplers[i]);
    }
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

void GLStateManager::PushShaderProgram()
{
    shaderState_.boundProgramStack.push(shaderState_.boundProgram);
}

void GLStateManager::PopShaderProgram()
{
    BindShaderProgram(shaderState_.boundProgramStack.top());
    shaderState_.boundProgramStack.pop();
}


/*
 * ======= Private: =======
 */

void GLStateManager::AssertExtViewportArray()
{
    if (!HasExtension(GLExt::ARB_viewport_array))
        throw std::runtime_error("renderer does not support viewport, depth-range, and scissor arrays");
}

void GLStateManager::SetActiveTextureLayer(unsigned int layer)
{
    textureState_.activeTexture = layer;
    activeTextureLayer_ = &(textureState_.layers[textureState_.activeTexture]);
}


} // /namespace LLGL



// ================================================================================
