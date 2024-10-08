/*
 * GLContextState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_CONTEXT_STATE_H
#define LLGL_GL_CONTEXT_STATE_H


#include <LLGL/Export.h>
#include "../OpenGL.h"
#include "GLState.h"


namespace LLGL
{


// Structure with all information about the state of an OpenGL context that can be managed by GLStateManager.
struct GLContextState
{
    static constexpr GLuint numTextureLayers    = 32;
    static constexpr GLuint numImageUnits       = 8;
    static constexpr GLuint numCaps             = static_cast<GLuint>(GLState::Num);
    static constexpr GLuint numBufferTargets    = static_cast<GLuint>(GLBufferTarget::Num);
    static constexpr GLuint numFboTargets       = static_cast<GLuint>(GLFramebufferTarget::Num);
    static constexpr GLuint numTextureTargets   = static_cast<GLuint>(GLTextureTarget::Num);

    #if LLGL_GL_ENABLE_VENDOR_EXT
    static constexpr GLuint numCapsExt          = static_cast<GLuint>(GLStateExt::Num);
    #endif

    // Rasterizer state
    #if LLGL_OPENGL
    GLenum          polygonMode                         = GL_FILL;
    #endif
    GLfloat         offsetFactor                        = 0.0f;
    GLfloat         offsetUnits                         = 0.0f;
    GLfloat         offsetClamp                         = 0.0f;
    GLenum          cullFace                            = GL_BACK;
    GLenum          frontFace                           = GL_CCW;
    GLint           patchVertices                       = 0;
    GLfloat         lineWidth                           = 1.0f;

    // Depth-stencil state
    GLenum          depthFunc                           = GL_LESS;
    GLboolean       depthMask                           = GL_TRUE;

    // Blend state
    GLfloat         blendColor[4]                       = { 0.0f, 0.0f, 0.0f, 0.0f };
    #if LLGL_OPENGL
    GLenum          logicOpCode                         = GL_COPY;
    #endif
    #if LLGL_PRIMITIVE_RESTART
    GLuint          primitiveRestartIndex               = 0;
    #endif

    // Clip control
    GLenum          clipOrigin                          = GL_LOWER_LEFT;
    #if LLGL_GLEXT_CLIP_CONTROL
    GLenum          clipDepthMode                       = GL_NEGATIVE_ONE_TO_ONE;
    #else
    GLenum          clipDepthMode                       = 0;
    #endif

    // Capabilities
    bool            capabilities[numCaps]               = {};

    #if LLGL_GL_ENABLE_VENDOR_EXT

    struct ExtensionState
    {
        GLenum      cap     = 0;
        bool        enabled = false;
    };

    ExtensionState  capabilitiesExt[numCapsExt]         = {};

    #endif // /LLGL_GL_ENABLE_VENDOR_EXT

    // Pixel store
    GLPixelStore    pixelStorePack;
    GLPixelStore    pixelStoreUnpack;

    // Buffers
    GLuint          boundBuffers[numBufferTargets]      = {};

    // Framebuffer Objects (FBO)
    GLuint          boundFramebuffers[numFboTargets]    = {};

    // Renerbuffer Objects (RBO)
    GLuint          boundRenderbuffer                   = 0;

    // Textures
    struct TextureLayer
    {
        GLuint      boundTextures[numTextureTargets]    = {};
    };

    GLuint          activeTexture                       = 0; // This refers to the zero-based index, not GL_TEXTURE1..GL_TEXTURE31
    TextureLayer    textureLayers[numTextureLayers];

    // Images
    //GLImageUnit     imageUnits[numImageUnits]           = {};

    // Vertex Array Objects (VAO)
    GLuint          boundVertexArray                    = 0;
    GLuint          boundElementArrayBuffer             = 0;

    // Programs
    GLuint          boundProgram                        = 0;
    GLuint          boundProgramPipeline                = 0;

    // Samplers
    GLuint          boundSamplers[numTextureLayers]     = {};

    // Transform-feedback
    GLuint          boundTransformFeedback              = 0;
};


// Querys the entire context state from OpenGL.
LLGL_EXPORT void GLGetContextState(GLContextState& outContextState);

// Sets the entire context state to OpenGL.
LLGL_EXPORT void GLSetContextState(const GLContextState& inContextState);


} // /namespace LLGL


#endif



// ================================================================================
