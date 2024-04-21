/*
 * DXCore.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DX_CORE_H
#define LLGL_DX_CORE_H


#include <LLGL/Utils/ColorRGBA.h>
#include <LLGL/RenderSystemFlags.h>
#include "../VideoAdapter.h"
#include <LLGL/ImageFlags.h>
#include "ComPtr.h"
#include <dxgi.h>
#include <string>
#include <vector>
#include <Windows.h>
#include <d3dcommon.h>


namespace LLGL
{


/* ----- Functions ----- */

// Returns the DLL instance handle of this module.
HINSTANCE DXGetDllHandle();

// Returns a string representation for the specified HRESULT error code.
const char* DXErrorToStrOrHex(HRESULT hr);

// Traps the runtime if 'hr' is not S_OK.
void DXThrowIfFailed(HRESULT hr, const char* info);

// Traps the runtime if 'hr' is not S_OK, with an info about the failed type cast from a COM pointer.
void DXThrowIfCastFailed(HRESULT hr, const char* interfaceName, const char* contextInfo = nullptr);

// Traps the runtime if 'hr' is not S_OK, with an info about the failed interface creation.
void DXThrowIfCreateFailed(HRESULT hr, const char* interfaceName, const char* contextInfo = nullptr);

// Traps the runtime if 'hr' is not S_OK, with an info about the failed function call.
void DXThrowIfInvocationFailed(HRESULT hr, const char* funcName, const char* contextInfo = nullptr);

// Returns the specified value as a DirectX BOOL type.
BOOL DXBoolean(bool value);

// Returns the blob data as string.
std::string DXGetBlobString(ID3DBlob* blob);

// Returns the blob data as char vector.
std::vector<char> DXGetBlobData(ID3DBlob* blob);

// Returns a blob and copies the specified data into the blob.
ComPtr<ID3DBlob> DXCreateBlob(const void* data, std::size_t size);

// Returns a blob and copies the specified data into the blob.
ComPtr<ID3DBlob> DXCreateBlob(const std::vector<char>& data);

// Returns a blob that was created from a resource (*.rc files).
ComPtr<ID3DBlob> DXCreateBlobFromResource(int resourceID);

// Returns the default list of supported D3D texture formats.
void DXGetDefaultSupportedTextureFormats(Format* outFormats, std::size_t* outNumFormats);

// Returns the compiler flags for the 'ShaderCompileFlags' enumeration values for the DirectX Effects Compiler (FXC).
UINT DXGetFxcCompilerFlags(int flags);

// Converts the adapter descriptor to video adapter information.
void DXConvertVideoAdapterInfo(IDXGIAdapter* adapter, const DXGI_ADAPTER_DESC& inDesc, VideoAdapterInfo& outInfo);

// Returns the video adapter descriptor from the specified DXGI adapter.
VideoAdapterInfo DXGetVideoAdapterInfo(IDXGIFactory* factory, long preferredAdapterFlags = 0, IDXGIAdapter** outPreferredAdatper = nullptr);

// Returns the format for the specified signature parameter type (by its component type and mask).
Format DXGetSignatureParameterType(D3D_REGISTER_COMPONENT_TYPE componentType, BYTE componentMask);

// Returns a suitable DXGI format for the specified depth-stencil mode.
DXGI_FORMAT DXPickDepthStencilFormat(int depthBits, int stencilBits);

// Returns true if the specified DXGI swap-chain is in fullscreen mode.
bool DXGetFullscreenState(IDXGISwapChain* swapChain);


} // /namespace LLGL


#endif



// ================================================================================
