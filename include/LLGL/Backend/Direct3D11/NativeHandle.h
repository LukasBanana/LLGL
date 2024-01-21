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
    //! COM pointer to the native Direct3D device.
    ID3D11Device*           device;

    //! COM pointer to the native Direct3D device context.
    ID3D11DeviceContext*    deviceContext;
};

struct CommandBufferNativeHandle
{
    /**
    \brief COM pointer to the native Direct3D device context.
    \remarks This is either the main device context from the RenderSystem or a deferred device context for a specific command buffer.
    \see RenderSystemNativeHandle::deviceContext
    */
    ID3D11DeviceContext* deviceContext;
};


} // /namespace Direct3D11

} // /namespace LLGL


#endif



// ================================================================================
