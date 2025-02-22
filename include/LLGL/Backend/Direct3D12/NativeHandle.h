/*
 * NativeHandle.h (Direct3D12)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DIRECT3D12_NATIVE_HANDLE_H
#define LLGL_DIRECT3D12_NATIVE_HANDLE_H


#include <dxgi1_4.h>
#include <d3d12.h>


namespace LLGL
{

namespace Direct3D12
{


/**
\brief Native handle structure for the Direct3D 12 render system.
\see RenderSystem::GetNativeHandle
\see RenderSystemDescriptor::nativeHandle
*/
struct RenderSystemNativeHandle
{
    /**
    \brief COM pointer to the DXGI factory version 4.
    \remarks Since Direct3D 12, the factory can no longer be backtracked from the device object that was used to create it.
    For LLGL to retrieve the adapter information, this factory must be of type \c IDXGIFactory4.
    \see https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_4/nf-dxgi1_4-idxgifactory4-enumadapterbyluid#remarks
    */
    IDXGIFactory4*      factory;

    //! COM pointer to the native Direct3D device.
    ID3D12Device*       device;

    //! COM pointer to the native Direct3D command queue.
    ID3D12CommandQueue* commandQueue;
};

/**
\brief Native handle structure for the Direct3D 12 command buffer.
\see CommandBuffer::GetNativeHandle
*/
struct CommandBufferNativeHandle
{
    //! COM pointer to the native Direct3D command list.
    ID3D12GraphicsCommandList* commandList;
};

/**
\brief Native Direct3D 12 resource type enumeration.
\see ResourceNativeHandle::type
*/
enum class ResourceNativeType
{
    /**
    \brief Native Direct3D resource type for buffers and textures.
    \see ResourceNativeHandle::resource
    */
    Resource,

    /**
    \brief Sampler-state descriptor.
    \see ResourceNativeHandle::samplerDesc
    */
    SamplerDescriptor,
};

/**
\brief Native handle structure for a Direct3D 12 resource.
\see Resource::GetNativeHandle
*/
struct ResourceNativeHandle
{
    struct NativeResource
    {
        /**
        \brief COM pointer to the native Direct3D resource.
        \remarks Here is an example how to distinguish this pointer between buffers and textures:
        \code
        LLGL::Direct3D12::ResourceNativeHandle myResourceNativeHandle;
        if (myResource->GetNativeHandle(&myResourceNativeHandle, sizeof(myResourceNativeHandle))) {
            if (myResourceNativeHandle.type == LLGL::Direct3D12::ResourceNativeType::Resource) {
                D3D12_RESOURCE_DESC d3dResourceDesc = myResourceNativeHandle.resource->GetDesc();
                switch (d3dResourceDesc.Dimension) {
                    case D3D12_RESOURCE_DIMENSION_BUFFER:
                        ...
                    case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
                        ...
                }
                myResourceNativeHandle.resource->Release(); // Release after use
            }
        }
        \endcode
        */
        ID3D12Resource*         resource;

        /**
        \brief Bitmask of resource states this resource is currently in.
        \remarks If the resource is transitioned into a different state after it has been retrieved via Resource::GetNativeHandle,
        it must be transitioned back into exactly this state before LLGL can use it again.
        */
        D3D12_RESOURCE_STATES   resourceState;
    };

    struct NativeSamplerDescriptor
    {
        /**
        \brief Native sampler-state descriptor.
        \remarks Direct3D12 native sampler descriptor.
        */
        D3D12_SAMPLER_DESC      samplerDesc;
    };

    /**
    \brief Specifies the native resource type.
    \remarks This allows to distinguish a resource between native resources and sampler-state descriptors.
    */
    ResourceNativeType type;

    union
    {
        //! Native Direct3D 12 resource.
        NativeResource          resource;

        //! Native Direct3D 12 sampler descriptor.
        NativeSamplerDescriptor samplerDesc;
    };
};


} // /namespace Direct3D12

} // /namespace LLGL


#endif



// ================================================================================
