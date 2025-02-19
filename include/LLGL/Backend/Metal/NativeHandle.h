/*
 * NativeHandle.h (Metal)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_METAL_NATIVE_HANDLE_H
#define LLGL_METAL_NATIVE_HANDLE_H

#ifdef __OBJC__
#import <Metal/Metal.h>
#endif


namespace LLGL
{

namespace Metal
{


/**
\brief Native handle structure for the Metal render system.
\see RenderSystem::GetNativeHandle
\see RenderSystemDescriptor::nativeHandle
*/
struct RenderSystemNativeHandle
{
#ifdef __OBJC__
    id<MTLDevice> device;
#else
    void* device;
#endif
};

/**
\brief Native handle structure for the Metal command buffer.
\see CommandBuffer::GetNativeHandle
*/
struct CommandBufferNativeHandle
{
#ifdef __OBJC__
    id<MTLCommandBuffer> commandBuffer;
#else
    void* commandBuffer;
#endif
};

/**
\brief Native Metal resource type enumeration.
\see ResourceNativeHandle::type
*/
enum class ResourceNativeType
{
    /**
    \brief Native Metal MTLBuffer resource.
    \remarks ResourceNativeHandle::buffer.
    */
    Buffer,

    /**
    \brief Native Metal MTLTexture resource.
    \remarks ResourceNativeHandle::texture.
    */
    Texture,

    /**
    \brief Native Metal MTLSamplerState resource.
    \remarks ResourceNativeHandle::samplerState.
    */
    SamplerState,
};

/**
\brief Native handle structure for a Metal resource.
\see Resource::GetNativeHandle
*/
struct ResourceNativeHandle
{
    /**
    \brief Specifies the native resource type.
    \remarks This allows to distinguish a resource between native resources and sampler-state descriptors.
    */
    ResourceNativeType type;
    
    union
    {
#ifdef __OBJC__
        //! Specifies the native Metal MTLBuffer object.
        id<MTLBuffer>       buffer;

        //! Specifies the native Metal MTLTexture object.
        id<MTLTexture>      texture;

        //! Specifies the native Metal MTLSamplerState object.
        id<MTLSamplerState> samplerState;
#else
        //! Specifies the native Metal MTLBuffer object.
        void* buffer;

        //! Specifies the native Metal MTLTexture object.
        void* texture;

        //! Specifies the native Metal MTLSamplerState object.
        void* samplerState;
#endif
    };
};


} // /namespace Metal

} // /namespace LLGL


#endif



// ================================================================================
