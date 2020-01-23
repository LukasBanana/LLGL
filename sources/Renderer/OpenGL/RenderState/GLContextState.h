/*
 * GLContextState.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_CONTEXT_STATE_H
#define LLGL_GL_CONTEXT_STATE_H


#include "GLState.h"


namespace LLGL
{


// Structure with all information about the state of an OpenGL context that can be managed by GLStateManager.
struct GLContextState
{
    static const GLuint numTextureLayers    = 32;
    static const GLuint numImageUnits       = 8;
    static const GLuint numCaps             = static_cast<GLuint>(GLState::Num);
    static const GLuint numBufferTargets    = static_cast<GLuint>(GLBufferTarget::Num);
    static const GLuint numFboTargets       = static_cast<GLuint>(GLFramebufferTarget::Num);
    static const GLuint numTextureTargets   = static_cast<GLuint>(GLTextureTarget::Num);

    #ifdef LLGL_GL_ENABLE_VENDOR_EXT
    static const GLuint numCapsExt          = static_cast<GLuint>(GLStateExt::Num);
    #endif

    // Rasterizer state
    GLenum          polygonMode                         = GL_FILL;
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
    GLboolean       cachedDepthMask                     = GL_TRUE;

    // Blend state
    GLfloat         blendColor[4]                       = { 0.0f, 0.0f, 0.0f, 0.0f };
    GLenum          logicOpCode                         = GL_COPY;

    // Capabilities
    bool            capabilities[numCaps];

    #ifdef LLGL_GL_ENABLE_VENDOR_EXT

    struct ExtensionState
    {
        GLenum      cap     = 0;
        bool        enabled = false;
    };

    ExtensionState  capabilitiesExt[numCapsExt];

    #endif

    // Pixel store
    GLPixelStore    pixelStorePack;
    GLPixelStore    pixelStoreUnpack;

    // Buffers
    GLuint          boundBuffers[numBufferTargets];

    // Framebuffer Objects (FBO)
    GLuint          boundFramebuffers[numFboTargets];

    // Renerbuffer Objects (RBO)
    GLuint          boundRenderbuffer                   = 0;

    // Textures
    struct GLTextureLayer
    {
        GLuint      boundTextures[numTextureTargets];
    };

    GLuint          activeTexture                       = 0;
    GLTextureLayer  textureLayers[numTextureLayers];

    // Images
    struct GLImageUnit
    {
        GLuint      texture;
        GLenum      format;
        GLenum      access;
    };

    GLImageUnit     imageUnits[numImageUnits];

    // Vertex Array Objects (VAO)
    GLuint          boundVertexArray                    = 0;
    GLuint          boundElementArrayBuffer             = 0;

    // Programs
    GLuint          boundProgram                        = 0;

    // Samplers
    GLuint          boundSamplers[numTextureLayers];
};


} // /namespace LLGL


#endif



// ================================================================================
