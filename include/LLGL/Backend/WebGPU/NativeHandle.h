/*
 * NativeHandle.h (WebGPU)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WEBGPU_NATIVE_HANDLE_H
#define LLGL_WEBGPU_NATIVE_HANDLE_H


#include <cstdint>
#include <webgpu/webgpu.h>


namespace LLGL
{

namespace WebGPU
{


/**
\brief Native handle structure for the WebGPU render system.
\see RenderSystem::GetNativeHandle
\see RenderSystemDescriptor::nativeHandle
*/
struct RenderSystemNativeHandle
{
    //! Native handle to the WebGPU instance.
    WGPUInstance        instance;

    //! Native handle to the WebGPU adapter.
    WGPUAdapter         adapter;

    //! Native handle to the logical WebGPU device.
    WGPUDevice          device;

    //! Native handle to the command queue.
    WGPUQueue           queue;
};

/**
\brief Native handle structure for the Vulkan command buffer.
\see CommandBuffer::GetNativeHandle
*/
struct CommandBufferNativeHandle
{
    /**
    \brief Native handle to the current command encoder.
    \remarks This can be null if the command buffer is not currently in recording mode.
    */
    WGPUCommandEncoder      commandEncoder;

    /**
    \brief Native handle to the current render-pass encoder.
    \remarks This can be null if the command buffer is not currently inside a render-pass section.
    */
    WGPURenderPassEncoder   renderPassEncoder;

    /**
    \brief Native handle to the current compute-pass encoder.
    \remarks This can be null if the command buffer does not currently bound to a compute pipeline.
    */
    WGPUComputePassEncoder  computePassEncoder;
};

/**
\brief Native WebGPU resource type enumeration.
\see ResourceNativeHandle::type
*/
enum class ResourceNativeType
{
    /**
    \brief Native WGPUBuffer type.
    \see ResourceNativeHandle::buffer
    */
    Buffer,

    /**
    \brief Native WGPUTexture type.
    \see ResourceNativeHandle::texture
    */
    Texture,

    /**
    \brief Native WGPUSampler type.
    \see ResourceNativeHandle::sampler
    */
    Sampler,
};

/**
\brief Native handle structure for a WebGPU resource.
\see Resource::GetNativeHandle
*/
struct ResourceNativeHandle
{
    /**
    \brief Specifies the native resource type.
    \remarks This allows to distinguish a resource between native Vulkan types.
    */
    ResourceNativeType  type;

    union
    {
        //! Buffer specific attriubtes.
        WGPUBuffer      buffer;

        //! Texture specific attriubtes.
        WGPUTexture     texture;

        //! Sampler specific attriubtes.
        WGPUSampler     sampler;
    };
};


} // /namespace Vulkan

} // /namespace LLGL


#endif



// ================================================================================
