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
    id<MTLDevice>   device;
    #else
    void*           device;
    #endif
};

/**
\brief Native handle structure for the Metal command buffer.
\see CommandBuffer::GetNativeHandle
*/
struct CommandBufferNativeHandle
{
    /**
    \brief Specifies the native MTLCommandBuffer that is currently used.
    \remarks This command buffer is invalidated after each command recording.
    */
    #ifdef __OBJC__
    id<MTLCommandBuffer>        commandBuffer;
    #else
    void*                       commandBuffer;
    #endif

    /**
    \brief Specifies the native MTLCommandEncoder that is currently bound for command encoding.
    \remarks This should be cast to the respective subtype, such as MTLRenderCommandEncoder
    if CommandBuffer::GetNativeHandle is called inside a render-pass for instance.
    If no command encoder is currently bound, this field is null.
    */
    #ifdef __OBJC__
    id<MTLCommandEncoder>       commandEncoder;
    #else
    void*                       commandEncoder;
    #endif

    /**
    \brief Specifies the native MTLRenderPassDescriptor that is currently used.
    \remarks If CommandBuffer::GetNativeHandle was called outside a render pass, this field will be null.
    */
    #ifdef __OBJC__
    MTLRenderPassDescriptor*    renderPassDesc;
    #else
    void*                       renderPassDesc;
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
        //! Specifies the native Metal MTLBuffer object.
        #ifdef __OBJC__
        id<MTLBuffer>       buffer;
        #else
        void*               buffer;
        #endif

        //! Specifies the native Metal MTLTexture object.
        #ifdef __OBJC__
        id<MTLTexture>      texture;
        #else
        void*               texture;
        #endif

        //! Specifies the native Metal MTLSamplerState object.
        #ifdef __OBJC__
        id<MTLSamplerState> samplerState;
        #else
        void*               samplerState;
        #endif
    };
};


} // /namespace Metal

} // /namespace LLGL


#endif



// ================================================================================
