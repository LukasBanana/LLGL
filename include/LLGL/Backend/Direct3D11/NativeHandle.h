/*
 * NativeHandle.h (Direct3D11)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DIRECT3D11_NATIVE_HANDLE_H
#define LLGL_DIRECT3D11_NATIVE_HANDLE_H


#include <d3d11.h>


namespace LLGL
{

namespace Direct3D11
{


/**
\brief Native handle structure for the Direct3D 11 render system.
\see RenderSystem::GetNativeHandle
\see RenderSystemDescriptor::nativeHandle
*/
struct RenderSystemNativeHandle
{
    /**
    \brief COM pointer to the native Direct3D device.
    \remarks When passed to \c RenderSystemDescriptor::nativeHandle, the backend adopts this device
    and \c deviceContext. When this is \c nullptr, the backend creates the device itself and the
    \c preferredAdapterLuid and \c minFeatureLevel fields below may constrain that creation.
    When returned from \c RenderSystem::GetNativeHandle, this is the device the backend is using.
    */
    ID3D11Device*           device;

    //! COM pointer to the native Direct3D device context.
    ID3D11DeviceContext*    deviceContext;

    /**
    \brief Optional adapter LUID the backend must use when creating the device.
    \remarks Only consulted when \c device is null on input. A zero LUID (\c {0, 0}) means
    "no LUID constraint" and the backend picks an adapter via the usual preference flags.
    Primarily used by the OpenXR binding to honor \c xrGetD3D11GraphicsRequirementsKHR.
    Ignored by \c RenderSystem::GetNativeHandle.
    */
    LUID                    preferredAdapterLuid;

    /**
    \brief Optional minimum feature level the created device must satisfy.
    \remarks Only consulted when \c device is null on input. A value of \c 0 means "no minimum".
    When set, the backend's feature-level ladder is filtered to entries \c >= minFeatureLevel
    and device creation fails if no entry succeeds. Primarily used by the OpenXR binding to honor
    \c XrGraphicsRequirementsD3D11KHR::minFeatureLevel. Ignored by \c RenderSystem::GetNativeHandle.
    */
    D3D_FEATURE_LEVEL       minFeatureLevel;
};

/**
\brief Native handle structure for the Direct3D 11 command buffer.
\see CommandBuffer::GetNativeHandle
*/
struct CommandBufferNativeHandle
{
    /**
    \brief COM pointer to the native Direct3D device context.
    \remarks This is either the main device context from the RenderSystem or a deferred device context for a specific command buffer.
    \see RenderSystemNativeHandle::deviceContext
    */
    ID3D11DeviceContext* deviceContext;
};

/**
\brief Native handle structure for a Direct3D 11 resource.
\see Resource::GetNativeHandle
*/
struct ResourceNativeHandle
{
    /**
    \brief COM pointer to the native Direct3D device child.
    \remarks Use the \c IUnknown::QueryInterface function to determine what type this refers to:
    \code
    LLGL::Direct3D11::ResourceNativeHandle myResourceNativeHandle;
    if (myResource->GetNativeHandle(&myResourceNativeHandle, sizeof(myResourceNativeHandle))) {
        // Check for buffers and textures
        ID3D11Resource* d3dResource = nullptr;
        if (myResourceNativeHandle.deviceChild->QueryInterface(IID_PPV_ARGS(&d3dResource)) == S_OK) {
            D3D11_RESOURCE_DIMENSION d3dResourceDimension = D3D11_RESOURCE_DIMENSION_UNKNOWN;
            d3dResource->GetType(&d3dResourceDimension);
            switch (d3dResourceDimension) {
                case D3D11_RESOURCE_DIMENSION_BUFFER:
                    ...
                case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
                    ...
            }
            d3dResource->Release(); // Release after QueryInterface()
        }

        // Check for sampler-states
        ID3D11SamplerState* d3dSamplerState = nullptr;
        if (myResourceNativeHandle.deviceChild->QueryInterface(IID_PPV_ARGS(&d3dSamplerState)) == S_OK) {
            ...
            d3dSamplerState->Release(); // Release after QueryInterface()
        }

        myResourceNativeHandle.deviceChild->Release(); // Release after LLGL's GetNativeHandle()
    }
    \endcode
    */
    ID3D11DeviceChild* deviceChild;
};


} // /namespace Direct3D11

} // /namespace LLGL


#endif



// ================================================================================
