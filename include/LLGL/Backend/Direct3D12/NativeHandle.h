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
    IDXGIFactory4*  factory;

    //! COM pointer to the native Direct3D device.
    ID3D12Device*   device;
};

struct CommandBufferNativeHandle
{
    //! COM pointer to the native Direct3D command list.
    ID3D12GraphicsCommandList* commandList;
};


} // /namespace Direct3D12

} // /namespace LLGL


#endif



// ================================================================================
