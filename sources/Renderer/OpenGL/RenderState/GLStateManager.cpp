/*
 * GLStateManager.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLStateManager.h"
#include "../GLRenderContext.h"
#include "../GLRenderSystem.h"
#include "../GLExtensions.h"
#include "../../../Core/Helper.h"
#include "../../Assertion.h"


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

GLStateManager::GLStateManager(GLRenderSystem& renderSystem)
{
    /* Initialize all states with zero */
    Fill(renderState_.values, false);
    Fill(bufferState_.boundBuffers, 0);
    Fill(frameBufferState_.boundFrameBuffers, 0);
    Fill(samplerState_.boundSamplers, 0);

    for (auto& layer : textureState_.layers)
        Fill(layer.boundTextures, 0);

    activeTextureLayer_ = &(textureState_.layers[0]);

    #ifdef LLGL_GL_ENABLE_EXT

    /* Initialize extenstion states */
    auto InitStateExt = [&](GLStateExt state, const std::string& extensionName, GLenum cap)
    {
        auto idx = static_cast<std::size_t>(state);
        auto& val = renderStateExt_.values[idx];
        if (val.cap == 0 && renderSystem.HasExtension(extensionName))
        {
            val.cap     = cap;
            val.enabled = (glIsEnabled(cap) != GL_FALSE);
        }
    };

    #ifdef GL_NV_conservative_raster
    // see https://www.opengl.org/registry/specs/NV/conservative_raster.txt
    InitStateExt(GLStateExt::CONSERVATIVE_RASTERIZATION, "GL_NV_conservative_raster", GL_CONSERVATIVE_RASTERIZATION_NV);
    #endif

    #ifdef GL_INTEL_conservative_rasterization
    // see https://www.opengl.org/registry/specs/INTEL/conservative_rasterization.txt
    InitStateExt(GLStateExt::CONSERVATIVE_RASTERIZATION, "GL_INTEL_conservative_raster", GL_CONSERVATIVE_RASTERIZATION_INTEL);
    #endif

    #endif

    /* Make this to the active state manager */
    GLStateManager::active = this;
}

void GLStateManager::NotifyRenderTargetHeight(GLint height)
{
    renderTargetHeight_ = height;
}

void GLStateManager::SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state)
{
    /* Store previous and new graphics API dependent state */
    auto prevState = gfxDependentState_;
    gfxDependentState_ = state;

    /* Update front face */
    if (prevState.invertFrontFace != state.invertFrontFace)
        SetFrontFace(commonState_.frontFace);
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

#ifdef LLGL_GL_ENABLE_EXT

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

void GLStateManager::SetViewports(std::vector<GLViewport>& viewports)
{
    if (viewports.size() == 1)
    {
        auto& vp = viewports.front();

        if (emulateClipControl_ && !gfxDependentState_.invertFrontFace)
            AdjustViewport(vp);

        glViewport(
            static_cast<GLint>(vp.x),
            static_cast<GLint>(vp.y),
            static_cast<GLsizei>(vp.width),
            static_cast<GLsizei>(vp.height)
        );
    }
    else if (viewports.size() > 1 && glViewportArrayv)
    {
        if (emulateClipControl_ && !gfxDependentState_.invertFrontFace)
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
}

void GLStateManager::SetDepthRanges(std::vector<GLDepthRange>& depthRanges)
{
    if (depthRanges.size() == 1)
    {
        const auto& dr = depthRanges.front();
        glDepthRange(dr.minDepth, dr.maxDepth);
    }
    else if (depthRanges.size() > 1 && glDepthRangeArrayv)
    {
        glDepthRangeArrayv(
            0,
            static_cast<GLsizei>(depthRanges.size()),
            reinterpret_cast<const GLdouble*>(depthRanges.data())
        );
    }
}

//private
void GLStateManager::AdjustScissor(GLScissor& scissor)
{
    scissor.y = renderTargetHeight_ - scissor.height - scissor.y;
}

void GLStateManager::SetScissors(std::vector<GLScissor>& scissors)
{
    if (scissors.size() == 1)
    {
        auto& sc = scissors.front();

        if (emulateClipControl_)
            AdjustScissor(sc);

        glScissor(sc.x, sc.y, sc.width, sc.height);
    }
    else if (scissors.size() > 1 && glScissorArrayv)
    {
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
}

void GLStateManager::SetBlendStates(const std::vector<GLBlend>& blendStates, bool blendEnabled)
{
    if (blendStates.size() == 1)
    {
        /* Set blend state only for the single draw buffer */
        const auto& state = blendStates.front();

        if (commonState_.colorMask != state.colorMask)
        {
            commonState_.colorMask = state.colorMask;
            glColorMask(state.colorMask.r, state.colorMask.g, state.colorMask.b, state.colorMask.a);
        }

        if (blendEnabled)
            glBlendFuncSeparate(state.srcColor, state.destColor, state.srcAlpha, state.destAlpha);
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
    if (glBlendFuncSeparatei != nullptr && glColorMaski != nullptr)
    {
        glColorMaski(drawBuffer, state.colorMask.r, state.colorMask.g, state.colorMask.b, state.colorMask.a);
        if (blendEnabled)
            glBlendFuncSeparatei(drawBuffer, state.srcColor, state.destColor, state.srcAlpha, state.destAlpha);
    }
    else
    {
        glDrawBuffer(drawBuffer);
        glColorMask(state.colorMask.r, state.colorMask.g, state.colorMask.b, state.colorMask.a);
        if (blendEnabled)
            glBlendFuncSeparate(state.srcColor, state.destColor, state.srcAlpha, state.destAlpha);
    }
}

void GLStateManager::SetClipControl(GLenum origin, GLenum depth)
{
    /*if (glClipControl)
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
    /* Check if mode must be inverted */
    if (gfxDependentState_.invertFrontFace)
    {
        switch (mode)
        {
            case GL_CW:
                mode = GL_CCW;
                break;
            case GL_CCW:
                mode = GL_CW;
                break;
        }
    }

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

/* ----- Buffer binding ----- */

void GLStateManager::BindBuffer(GLBufferTarget target, GLuint buffer)
{
    /* Only bind buffer if the buffer has changed */
    auto targetIdx = static_cast<std::size_t>(target);
    if (bufferState_.boundBuffers[targetIdx] != buffer)
    {
        bufferState_.boundBuffers[targetIdx] = buffer;
        glBindBuffer(bufferTargetsMap[targetIdx], buffer);
    }
}

void GLStateManager::BindBufferBase(GLBufferTarget target, GLuint index, GLuint buffer)
{
    /* Always bind buffer with a base index */
    auto targetIdx = static_cast<std::size_t>(target);
    bufferState_.boundBuffers[targetIdx] = buffer;
    glBindBufferBase(bufferTargetsMap[targetIdx], index, buffer);
}

void GLStateManager::BindVertexArray(GLuint buffer)
{
    /* Always bind vertex array */
    glBindVertexArray(buffer);
    bufferState_.boundBuffers[static_cast<std::size_t>(GLBufferTarget::ARRAY_BUFFER)] = 0;
    bufferState_.boundBuffers[static_cast<std::size_t>(GLBufferTarget::ELEMENT_ARRAY_BUFFER)] = 0;
}

void GLStateManager::ForcedBindBuffer(GLBufferTarget target, GLuint buffer)
{
    /* Always bind buffer with a forced binding */
    auto targetIdx = static_cast<std::size_t>(target);
    bufferState_.boundBuffers[targetIdx] = buffer;
    glBindBuffer(bufferTargetsMap[targetIdx], buffer);
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

void GLStateManager::BindBuffer(const GLVertexBuffer& vertexBuffer)
{
    BindBuffer(GLBufferTarget::ARRAY_BUFFER, vertexBuffer.hwBuffer.GetID());
}

void GLStateManager::BindBuffer(const GLIndexBuffer& indexBuffer)
{
    BindBuffer(GLBufferTarget::ELEMENT_ARRAY_BUFFER, indexBuffer.hwBuffer.GetID());
}

void GLStateManager::BindBuffer(const GLConstantBuffer& constantBuffer)
{
    BindBuffer(GLBufferTarget::UNIFORM_BUFFER, constantBuffer.hwBuffer.GetID());
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
        textureState_.activeTexture = layer;
        activeTextureLayer_ = &(textureState_.layers[textureState_.activeTexture]);
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

void GLStateManager::ForcedBindTexture(GLTextureTarget target, GLuint texture)
{
    /* Always bind texutre with a forced binding */
    auto targetIdx = static_cast<std::size_t>(target);
    activeTextureLayer_->boundTextures[targetIdx] = texture;
    glBindTexture(textureTargetsMap[targetIdx], texture);
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

void GLStateManager::ForcedBindTexture(const GLTexture& texture)
{
    ForcedBindTexture(GLStateManager::GetTextureTarget(texture.GetType()), texture.GetID());
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


} // /namespace LLGL



// ================================================================================
