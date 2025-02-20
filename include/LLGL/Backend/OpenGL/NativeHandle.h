/*
 * NativeHandle.h (OpenGL)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENGL_NATIVE_HANDLE_H
#define LLGL_OPENGL_NATIVE_HANDLE_H


#include <LLGL/Platform/Platform.h>

#if defined(LLGL_OS_WIN32)
#   include <GL/GL.h>
#   include <LLGL/Backend/OpenGL/Win32/Win32NativeHandle.h>
#elif defined(LLGL_OS_MACOS)
#   include <AvailabilityMacros.h>
#   if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
#       include <OpenGL/gl3.h>
#   else
#       include <OpenGL/gl.h>
#   endif
#   include <LLGL/Backend/OpenGL/MacOS/MacOSNativeHandle.h>
#elif defined(LLGL_OS_LINUX)
#   include <GL/gl.h>
#   include <LLGL/Backend/OpenGL/Linux/LinuxNativeHandle.h>
#elif defined(LLGL_OS_IOS)
#   include <OpenGLES/ES3/gl.h>
#   ifdef __OBJC__
#       include <LLGL/Backend/OpenGL/IOS/IOSNativeHandle.h>
#   endif
#elif defined(LLGL_OS_ANDROID)
#   include <GLES3/gl3.h>
#   include <LLGL/Backend/OpenGL/Android/AndroidNativeHandle.h>
#elif defined(LLGL_OS_WASM)
#   include <webgl/webgl2.h>
#   include <LLGL/Backend/OpenGL/Wasm/WasmNativeHandle.h>
#endif


namespace LLGL
{

namespace OpenGL
{


/**
\brief Native OpenGL resource type enumeration.
\see ResourceNativeHandle::type
*/
enum class ResourceNativeType
{
    /**
    \brief Default buffer resource.
    \remarks Its identifier was created with \c glGenBuffers.
    */
    Buffer,

    /**
    \brief Default texture resource.
    \remarks Its identifier was created with \c glGenTextures.
    */
    Texture,

    /**
    \brief Sampler-state resource.
    \remarks Its identifier was created with \c glGenSamplers if the OpenGL extension \c GL_ARB_sampler_objects is available.
    */
    Sampler,

    /**
    \brief Default renderbuffer resource.
    \remarks Its identifier was created with \c glGenRenderbuffers.
    \remarks This is used for depth-stencil textures that cannot be read or written directly by shaders.
    */
    Renderbuffer,

    /**
    \brief Immutable buffer resource.
    \remarks Its identifier was created with \c glCreateBuffers if the OpenGL extension \c GL_ARB_direct_state_access is available.
    */
    ImmutableBuffer,

    /**
    \brief Immutable texture resource.
    \remarks Its identifier was created with \c glCreateTextures if the OpenGL extension \c GL_ARB_direct_state_access is available.
    */
    ImmutableTexture,

    /**
    \brief Immutable renderbuffer resource.
    \remarks Its identifier was created with \c glCreateRenderbuffers if the OpenGL extension \c GL_ARB_direct_state_access is available.
    \remarks This is used for depth-stencil textures that cannot be read or written directly by shaders.
    */
    ImmutableRenderbuffer,

    /**
    \brief Emulated sampler-state resource.
    \remarks This does \e not provide an identifier and is only used when the OpenGL extension \c GL_ARB_sampler_objects is \e not available.
    */
    EmulatedSampler,
};

/**
\brief Native handle structure for an OpenGL resource.
\see Resource::GetNativeHandle
*/
struct ResourceNativeHandle
{
    struct NativeBuffer
    {
        /**
        \brief Secondary identifier for a texture buffer. 0 if unused.
        \remarks This refers to an OpenGL texture for the \c GL_TEXTURE_BUFFER target.
        Its data is pointing to the buffer specified by the primary resource identifier.
        */
        GLuint textureId;
    };

    struct NativeTexture
    {
        /**
        \brief Specifies the texture extent of the texture.
        \remarks This is provided because OpenGLES does not support to query the texture dimensions as Desktop OpenGL does.
        So LLGL stores it at texture creation time.
        */
        GLint extent[3];

        /**
        \brief Specifies the texture sample count for multi-sampled textures.
        \remarks This is provided because OpenGLES does not support to query the texture dimensions as Desktop OpenGL does.
        So LLGL stores it at texture creation time.
        */
        GLint samples;
    };

    /**
    \brief Specifies the native resource type.
    \remarks This allows to distinguish a resource between mutable and immutable types.
    */
    ResourceNativeType  type;

    /**
    \brief Specifies the primary resource identifier.
    \remarks This value comes from either \c glGenBuffers, \c glCreateBuffers, \c glGenTextures,
    \c glCreateTextures, \c glGenRenderbuffers, \c glCreateRenderbuffers, or \c glGenSamplers.
    This can also be 0 if it refers to an emulated type such as emulated sampler states for legacy OpenGL (i.e. 2.0 and older).
    */
    GLuint              id;

    union
    {
        //! Buffer specific attributes.
        NativeBuffer    buffer;

        //! Texture specific attributes.
        NativeTexture   texture;
    };
};


} // /namespace OpenGL

} // /namespace LLGL


#endif



// ================================================================================
