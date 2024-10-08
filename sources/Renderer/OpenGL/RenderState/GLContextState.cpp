/*
 * GLContextState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLContextState.h"
#include "GLStateManager.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../../../Core/MacroUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Backend/OpenGL/NativeCommand.h>


namespace LLGL
{


constexpr GLuint GLContextState::numTextureLayers;
constexpr GLuint GLContextState::numImageUnits;
constexpr GLuint GLContextState::numCaps;
constexpr GLuint GLContextState::numBufferTargets;
constexpr GLuint GLContextState::numFboTargets;
constexpr GLuint GLContextState::numTextureTargets;

static void GLGetValue(GLenum pname, GLint& params)
{
    glGetIntegerv(pname, &params);
}

static void GLGetValue(GLenum pname, GLint* params)
{
    glGetIntegerv(pname, params);
}

static void GLGetValue(GLenum pname, GLenum& params)
{
    GLint value = 0;
    glGetIntegerv(pname, &value);
    params = static_cast<GLenum>(value);
}

static void GLGetValue(GLenum pname, GLfloat& params)
{
    glGetFloatv(pname, &params);
}

static void GLGetValue(GLenum pname, GLfloat* params)
{
    glGetFloatv(pname, params);
}

static void GLGetValue(GLenum pname, GLboolean& params)
{
    glGetBooleanv(pname, &params);
}

#ifndef GL_ARRAY_BUFFER_BINDING
#error GL_ARRAY_BUFFER_BINDING is not defined; Cannot build OpenGL backend!
#endif

static const GLenum g_bufferTargetBindings[] =
{
    GL_ARRAY_BUFFER_BINDING,

    #ifdef GL_ATOMIC_COUNTER_BUFFER_BINDING
    GL_ATOMIC_COUNTER_BUFFER_BINDING,
    #else
    0,
    #endif

    #ifdef GL_COPY_READ_BUFFER_BINDING
    GL_COPY_READ_BUFFER_BINDING,
    #else
    0,
    #endif

    #ifdef GL_COPY_WRITE_BUFFER_BINDING
    GL_COPY_WRITE_BUFFER_BINDING,
    #else
    0,
    #endif

    #ifdef GL_DISPATCH_INDIRECT_BUFFER_BINDING
    GL_DISPATCH_INDIRECT_BUFFER_BINDING,
    #else
    0,
    #endif

    #ifdef GL_DRAW_INDIRECT_BUFFER_BINDING
    GL_DRAW_INDIRECT_BUFFER_BINDING,
    #else
    0,
    #endif

    #ifdef GL_ELEMENT_ARRAY_BUFFER_BINDING
    GL_ELEMENT_ARRAY_BUFFER_BINDING,
    #else
    0,
    #endif

    #ifdef GL_PIXEL_PACK_BUFFER_BINDING
    GL_PIXEL_PACK_BUFFER_BINDING,
    #else
    0,
    #endif

    #ifdef GL_PIXEL_UNPACK_BUFFER_BINDING
    GL_PIXEL_UNPACK_BUFFER_BINDING,
    #else
    0,
    #endif

    #ifdef GL_QUERY_BUFFER_BINDING
    GL_QUERY_BUFFER_BINDING,
    #else
    0,
    #endif

    #ifdef GL_SHADER_STORAGE_BUFFER_BINDING
    GL_SHADER_STORAGE_BUFFER_BINDING,
    #else
    0,
    #endif

    #ifdef GL_TEXTURE_BUFFER_BINDING
    GL_TEXTURE_BUFFER_BINDING,
    #else
    0,
    #endif

    #ifdef GL_TRANSFORM_FEEDBACK_BUFFER_BINDING
    GL_TRANSFORM_FEEDBACK_BUFFER_BINDING,
    #else
    0,
    #endif

    #ifdef GL_UNIFORM_BUFFER_BINDING
    GL_UNIFORM_BUFFER_BINDING,
    #else
    0,
    #endif
};

static_assert(
    LLGL_ARRAY_LENGTH(g_bufferTargetBindings) == GLContextState::numBufferTargets,
    "Array length of 'g_bufferTargetBindings' must be equal to GLContextState::numBufferTargets"
);

static const GLenum g_textureTargetBindings[] =
{
    #ifdef GL_TEXTURE_BINDING_1D
    GL_TEXTURE_BINDING_1D,
    #else
    0,
    #endif

    #ifdef GL_TEXTURE_BINDING_2D
    GL_TEXTURE_BINDING_2D,
    #else
    0,
    #endif

    #ifdef GL_TEXTURE_BINDING_3D
    GL_TEXTURE_BINDING_3D,
    #else
    0,
    #endif

    #ifdef GL_TEXTURE_BINDING_1D_ARRAY
    GL_TEXTURE_BINDING_1D_ARRAY,
    #else
    0,
    #endif

    #ifdef GL_TEXTURE_BINDING_2D_ARRAY
    GL_TEXTURE_BINDING_2D_ARRAY,
    #else
    0,
    #endif

    #ifdef GL_TEXTURE_BINDING_RECTANGLE
    GL_TEXTURE_BINDING_RECTANGLE,
    #else
    0,
    #endif

    #ifdef GL_TEXTURE_BINDING_CUBE_MAP
    GL_TEXTURE_BINDING_CUBE_MAP,
    #else
    0,
    #endif

    #ifdef GL_TEXTURE_BINDING_CUBE_MAP_ARRAY
    GL_TEXTURE_BINDING_CUBE_MAP_ARRAY,
    #else
    0,
    #endif

    #ifdef GL_TEXTURE_BINDING_BUFFER
    GL_TEXTURE_BINDING_BUFFER,
    #else
    0,
    #endif

    #ifdef GL_TEXTURE_BINDING_2D_MULTISAMPLE
    GL_TEXTURE_BINDING_2D_MULTISAMPLE,
    #else
    0,
    #endif

    #ifdef GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY
    GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY,
    #else
    0,
    #endif
};

static_assert(
    LLGL_ARRAY_LENGTH(g_textureTargetBindings) == GLContextState::numTextureTargets,
    "Array length of 'g_textureTargetBindings' must be equal to GLContextState::numTextureTargets"
);

LLGL_EXPORT void GLGetContextState(GLContextState& outContextState)
{
    // Rasterizer state
    #if LLGL_OPENGL
    GLint polygonModes[2] = {};
    GLGetValue(GL_POLYGON_MODE, polygonModes);
    outContextState.polygonMode = polygonModes[0];
    #endif

    GLGetValue(GL_POLYGON_OFFSET_FACTOR,    outContextState.offsetFactor);
    GLGetValue(GL_POLYGON_OFFSET_UNITS,     outContextState.offsetUnits);

    #if LLGL_GLEXT_POLYGON_OFFSET_CLAMP
    if (HasExtension(GLExt::ARB_polygon_offset_clamp))
        GLGetValue(GL_POLYGON_OFFSET_CLAMP, outContextState.offsetClamp);
    #endif

    GLGetValue(GL_CULL_FACE_MODE,           outContextState.cullFace);
    GLGetValue(GL_FRONT_FACE,               outContextState.frontFace);

    #if LLGL_GLEXT_TESSELLATION_SHADER
    if (HasExtension(GLExt::ARB_tessellation_shader))
        GLGetValue(GL_PATCH_VERTICES,       outContextState.patchVertices);
    #endif

    GLGetValue(GL_LINE_WIDTH,               outContextState.lineWidth);

    // Depth-stencil state
    GLGetValue(GL_DEPTH_FUNC,               outContextState.depthFunc);
    GLGetValue(GL_DEPTH_WRITEMASK,          outContextState.depthMask);

    // Blend state
    GLGetValue(GL_BLEND_COLOR,              outContextState.blendColor);
    #if LLGL_OPENGL
    GLGetValue(GL_LOGIC_OP_MODE,            outContextState.logicOpCode);
    #endif
    #if LLGL_PRIMITIVE_RESTART
    GLGetValue(GL_PRIMITIVE_RESTART_INDEX,  outContextState.primitiveRestartIndex);
    #endif

    // Clip control
    #if LLGL_GLEXT_CLIP_CONTROL
    if (HasExtension(GLExt::ARB_clip_control))
    {
        GLGetValue(GL_CLIP_ORIGIN,      outContextState.clipOrigin);
        GLGetValue(GL_CLIP_DEPTH_MODE,  outContextState.clipDepthMode);
    }
    #endif // /LLGL_GLEXT_CLIP_CONTROL

    // Capabilities
    for_range(i, GLContextState::numCaps)
    {
        const GLenum cap = GLStateManager::GetGLCapability(static_cast<GLState>(i));
        outContextState.capabilities[i] = (cap != 0 && glIsEnabled(cap) != GL_FALSE);
    }

    #ifdef LLGL_GL_ENABLE_VENDOR_EXT
    for_range(i, GLContextState::numCapsExt)
    {
        const GLenum cap = outContextState.capabilitiesExt[i].cap;
        outContextState.capabilitiesExt[i].enabled = (cap != 0 && glIsEnabled(cap) != GL_FALSE);
    }
    #endif // /LLGL_GL_ENABLE_VENDOR_EXT

    // Pixel store
    GLGetValue(GL_PACK_ROW_LENGTH,      outContextState.pixelStorePack.rowLength);
    #ifdef LLGL_OPENGL
    GLGetValue(GL_PACK_IMAGE_HEIGHT,    outContextState.pixelStorePack.imageHeight);
    #endif
    GLGetValue(GL_PACK_ALIGNMENT,       outContextState.pixelStorePack.alignment);

    GLGetValue(GL_UNPACK_ROW_LENGTH,    outContextState.pixelStoreUnpack.rowLength);
    GLGetValue(GL_UNPACK_IMAGE_HEIGHT,  outContextState.pixelStoreUnpack.imageHeight);
    GLGetValue(GL_UNPACK_ALIGNMENT,     outContextState.pixelStoreUnpack.alignment);

    // Buffers
    for_range(target, GLContextState::numBufferTargets)
    {
        if (g_bufferTargetBindings[target] != 0)
            GLGetValue(g_bufferTargetBindings[target], outContextState.boundBuffers[target]);
    }

    #if !LLGL_GL_ENABLE_OPENGL2X

    // Framebuffer Objects (FBO)
    GLGetValue(GL_DRAW_FRAMEBUFFER_BINDING, outContextState.boundFramebuffers[static_cast<int>(GLFramebufferTarget::DrawFramebuffer)]);
    GLGetValue(GL_READ_FRAMEBUFFER_BINDING, outContextState.boundFramebuffers[static_cast<int>(GLFramebufferTarget::ReadFramebuffer)]);
    outContextState.boundFramebuffers[static_cast<int>(GLFramebufferTarget::Framebuffer)] = outContextState.boundFramebuffers[static_cast<int>(GLFramebufferTarget::DrawFramebuffer)];

    // Renerbuffer Objects (RBO)
    GLGetValue(GL_RENDERBUFFER_BINDING, outContextState.boundRenderbuffer);

    #endif // /!LLGL_GL_ENABLE_OPENGL2X

    // Textures and samplers
    GLenum initialActiveTexture = GL_TEXTURE0;
    GLGetValue(GL_ACTIVE_TEXTURE, initialActiveTexture);

    for_range(layer, GLContextState::numTextureLayers)
    {
        const GLenum currentActiveTexture = GLStateManager::ToGLTextureLayer(layer);
        glActiveTexture(currentActiveTexture);

        #if LLGL_GLEXT_SAMPLER_OBJECTS
        GLGetValue(GL_SAMPLER_BINDING, outContextState.boundSamplers[layer]);
        #endif

        for_range(target, GLContextState::numTextureTargets)
        {
            if (g_textureTargetBindings[target] != 0)
                GLGetValue(g_textureTargetBindings[target], outContextState.textureLayers[layer].boundTextures[target]);
        }

        if (currentActiveTexture == initialActiveTexture)
            outContextState.activeTexture = layer;
    }

    glActiveTexture(initialActiveTexture);

    // Vertex Array Objects (VAO)
    #if LLGL_GLEXT_VERTEX_ARRAY_OBJECT
    GLGetValue(GL_VERTEX_ARRAY_BINDING, outContextState.boundVertexArray);
    #endif
    outContextState.boundElementArrayBuffer = outContextState.boundBuffers[static_cast<int>(GLBufferTarget::ElementArrayBuffer)]; //TODO: remove this redundancy

    // Programs
    #ifdef GL_ARB_shader_objects
    if (HasExtension(GLExt::ARB_shader_objects))
        GLGetValue(GL_CURRENT_PROGRAM, outContextState.boundProgram);
    #endif

    #if LLGL_GLEXT_SEPARATE_SHADER_OBJECTS
    if (HasExtension(GLExt::ARB_separate_shader_objects))
        GLGetValue(GL_PROGRAM_PIPELINE_BINDING, outContextState.boundProgramPipeline);
    #endif

    #if LLGL_GLEXT_TRNASFORM_FEEDBACK2
    if (HasExtension(GLExt::ARB_transform_feedback2))
        GLGetValue(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, outContextState.boundTransformFeedback);
    #endif
};


LLGL_EXPORT void GLSetContextState(const GLContextState& inContextState)
{
    // Rasterizer state
    #ifdef LLGL_OPENGL
    glPolygonMode(GL_FRONT_AND_BACK, inContextState.polygonMode);
    #endif

    #if LLGL_GLEXT_POLYGON_OFFSET_CLAMP
    if (HasExtension(GLExt::ARB_polygon_offset_clamp))
    {
        glPolygonOffsetClamp(
            inContextState.offsetFactor,
            inContextState.offsetUnits,
            inContextState.offsetClamp
        );
    }
    else
    #endif
    {
        glPolygonOffset(
            inContextState.offsetFactor,
            inContextState.offsetUnits
        );
    }

    glCullFace(inContextState.cullFace);
    glFrontFace(inContextState.frontFace);

    #if LLGL_GLEXT_TESSELLATION_SHADER
    if (HasExtension(GLExt::ARB_tessellation_shader))
        glPatchParameteri(GL_PATCH_VERTICES, inContextState.patchVertices);
    #endif

    glLineWidth(inContextState.lineWidth);

    // Depth-stencil state
    glDepthFunc(inContextState.depthFunc);
    glDepthMask(inContextState.depthMask);

    // Blend state
    glBlendColor(
        inContextState.blendColor[0],
        inContextState.blendColor[1],
        inContextState.blendColor[2],
        inContextState.blendColor[3]
    );

    #if LLGL_OPENGL
    glLogicOp(inContextState.logicOpCode);
    #endif

    #if LLGL_PRIMITIVE_RESTART
    glPrimitiveRestartIndex(inContextState.primitiveRestartIndex);
    #endif

    // Clip control
    #if LLGL_GLEXT_CLIP_CONTROL
    if (HasExtension(GLExt::ARB_clip_control))
    {
        glClipControl(
            inContextState.clipOrigin,
            inContextState.clipDepthMode
        );
    }
    #endif // /LLGL_GLEXT_CLIP_CONTROL

    // Capabilities
    for_range(i, GLContextState::numCaps)
    {
        const GLenum cap = GLStateManager::GetGLCapability(static_cast<GLState>(i));
        if (cap != 0)
        {
            if (inContextState.capabilities[i])
                glEnable(cap);
            else
                glDisable(cap);
        }
    }

    #ifdef LLGL_GL_ENABLE_VENDOR_EXT
    for_range(i, GLContextState::numCapsExt)
    {
        const GLenum cap = inContextState.capabilitiesExt[i].cap;
        if (cap != 0)
        {
            if (inContextState.capabilitiesExt[i].enabled)
                glEnable(cap);
            else
                glDisable(cap);
        }
    }
    #endif // /LLGL_GL_ENABLE_VENDOR_EXT

    // Pixel store
    glPixelStorei(GL_PACK_ROW_LENGTH,       inContextState.pixelStorePack.rowLength);
    #ifdef LLGL_OPENGL
    glPixelStorei(GL_PACK_IMAGE_HEIGHT,     inContextState.pixelStorePack.imageHeight);
    #endif
    glPixelStorei(GL_PACK_ALIGNMENT,        inContextState.pixelStorePack.alignment);

    glPixelStorei(GL_UNPACK_ROW_LENGTH,     inContextState.pixelStoreUnpack.rowLength);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT,   inContextState.pixelStoreUnpack.imageHeight);
    glPixelStorei(GL_UNPACK_ALIGNMENT,      inContextState.pixelStoreUnpack.alignment);

    // Buffers
    for_range(target, GLContextState::numBufferTargets)
    {
        if (g_bufferTargetBindings[target] != 0)
            glBindBuffer(GLStateManager::ToGLBufferTarget(static_cast<GLBufferTarget>(target)), inContextState.boundBuffers[target]);
    }

    #if !LLGL_GL_ENABLE_OPENGL2X

    // Framebuffer Objects (FBO)
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, inContextState.boundFramebuffers[static_cast<int>(GLFramebufferTarget::DrawFramebuffer)]);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, inContextState.boundFramebuffers[static_cast<int>(GLFramebufferTarget::ReadFramebuffer)]);

    // Renerbuffer Objects (RBO)
    glBindRenderbuffer(GL_RENDERBUFFER_BINDING, inContextState.boundRenderbuffer);

    #endif // /!LLGL_GL_ENABLE_OPENGL2X

    // Textures and samplers
    GLenum initialActiveTexture = GL_TEXTURE0;
    GLGetValue(GL_ACTIVE_TEXTURE, initialActiveTexture);

    for_range(layer, GLContextState::numTextureLayers)
    {
        const GLenum currentActiveTexture = GLStateManager::ToGLTextureLayer(layer);
        glActiveTexture(currentActiveTexture);

        #if LLGL_GLEXT_SAMPLER_OBJECTS
        glBindSampler(layer, inContextState.boundSamplers[layer]);
        #endif

        for_range(target, GLContextState::numTextureTargets)
        {
            glBindTexture(
                GLStateManager::ToGLTextureTarget(static_cast<GLTextureTarget>(target)),
                inContextState.textureLayers[layer].boundTextures[target]
            );
        }
    }

    glActiveTexture(initialActiveTexture);

    // Vertex Array Objects (VAO)
    #if LLGL_GLEXT_VERTEX_ARRAY_OBJECT
    glBindVertexArray(inContextState.boundVertexArray);
    #endif
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, inContextState.boundElementArrayBuffer);

    // Programs
    #ifdef GL_ARB_shader_objects
    if (HasExtension(GLExt::ARB_shader_objects))
        glUseProgram(inContextState.boundProgram);
    #endif

    #if LLGL_GLEXT_SEPARATE_SHADER_OBJECTS
    if (HasExtension(GLExt::ARB_separate_shader_objects))
        glBindProgramPipeline(inContextState.boundProgramPipeline);
    #endif

    #if LLGL_GLEXT_TRNASFORM_FEEDBACK2
    if (HasExtension(GLExt::ARB_transform_feedback2))
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, inContextState.boundTransformFeedback);
    #endif
}


} // /namespace LLGL



// ================================================================================
