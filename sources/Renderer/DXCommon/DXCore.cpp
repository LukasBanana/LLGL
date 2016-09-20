/*
 * DXCore.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../DXCommon/DXCore.h"
#include "../../Core/Helper.h"
#include "../../Core/HelperMacros.h"
#include <stdexcept>
#include <algorithm>


namespace LLGL
{


std::string DXErrorToStr(const HRESULT errorCode)
{
    switch (errorCode)
    {
        // see https://msdn.microsoft.com/en-us/library/windows/desktop/aa378137(v=vs.85).aspx
        LLGL_CASE_TO_STR( S_OK );
        LLGL_CASE_TO_STR( S_FALSE );
        LLGL_CASE_TO_STR( E_ABORT );
        LLGL_CASE_TO_STR( E_ACCESSDENIED );
        LLGL_CASE_TO_STR( E_FAIL );
        LLGL_CASE_TO_STR( E_HANDLE );
        LLGL_CASE_TO_STR( E_INVALIDARG );
        LLGL_CASE_TO_STR( E_NOINTERFACE );
        LLGL_CASE_TO_STR( E_NOTIMPL );
        LLGL_CASE_TO_STR( E_OUTOFMEMORY );
        LLGL_CASE_TO_STR( E_POINTER );
        LLGL_CASE_TO_STR( E_UNEXPECTED );

        // see https://msdn.microsoft.com/en-us/library/windows/desktop/bb509553(v=vs.85).aspx
        LLGL_CASE_TO_STR( DXGI_ERROR_DEVICE_HUNG );
        LLGL_CASE_TO_STR( DXGI_ERROR_DEVICE_REMOVED );
        LLGL_CASE_TO_STR( DXGI_ERROR_DEVICE_RESET );
        LLGL_CASE_TO_STR( DXGI_ERROR_DRIVER_INTERNAL_ERROR );
        LLGL_CASE_TO_STR( DXGI_ERROR_FRAME_STATISTICS_DISJOINT );
        LLGL_CASE_TO_STR( DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE );
        LLGL_CASE_TO_STR( DXGI_ERROR_INVALID_CALL );
        LLGL_CASE_TO_STR( DXGI_ERROR_MORE_DATA );
        LLGL_CASE_TO_STR( DXGI_ERROR_NONEXCLUSIVE );
        LLGL_CASE_TO_STR( DXGI_ERROR_NOT_CURRENTLY_AVAILABLE );
        LLGL_CASE_TO_STR( DXGI_ERROR_NOT_FOUND );
        LLGL_CASE_TO_STR( DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED );
        LLGL_CASE_TO_STR( DXGI_ERROR_REMOTE_OUTOFMEMORY );
        LLGL_CASE_TO_STR( DXGI_ERROR_WAS_STILL_DRAWING );
        LLGL_CASE_TO_STR( DXGI_ERROR_UNSUPPORTED );
        LLGL_CASE_TO_STR( DXGI_ERROR_ACCESS_LOST );
        LLGL_CASE_TO_STR( DXGI_ERROR_WAIT_TIMEOUT );
        LLGL_CASE_TO_STR( DXGI_ERROR_SESSION_DISCONNECTED );
        LLGL_CASE_TO_STR( DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE );
        LLGL_CASE_TO_STR( DXGI_ERROR_CANNOT_PROTECT_CONTENT );
        LLGL_CASE_TO_STR( DXGI_ERROR_ACCESS_DENIED );
        LLGL_CASE_TO_STR( DXGI_ERROR_NAME_ALREADY_EXISTS );
        LLGL_CASE_TO_STR( DXGI_ERROR_SDK_COMPONENT_MISSING );

        // see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476174(v=vs.85).aspx
        LLGL_CASE_TO_STR( D3D10_ERROR_FILE_NOT_FOUND );
        LLGL_CASE_TO_STR( D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS );
        LLGL_CASE_TO_STR( D3D11_ERROR_FILE_NOT_FOUND );
        LLGL_CASE_TO_STR( D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS );
        LLGL_CASE_TO_STR( D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS );
        LLGL_CASE_TO_STR( D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD );
        LLGL_CASE_TO_STR( D3D12_ERROR_ADAPTER_NOT_FOUND );
        LLGL_CASE_TO_STR( D3D12_ERROR_DRIVER_VERSION_MISMATCH );
    }
    return ToHex(errorCode);
}

void DXThrowIfFailed(const HRESULT errorCode, const std::string& info)
{
    if (FAILED(errorCode))
        throw std::runtime_error(info + " (error code = " + DXErrorToStr(errorCode) + ")");
}

template <typename Cont>
Cont GetBlobDataTmpl(ID3DBlob* blob)
{
    auto data = reinterpret_cast<const char*>(blob->GetBufferPointer());
    auto size = static_cast<std::size_t>(blob->GetBufferSize());

    Cont container;
    {
        container.resize(size);
        std::copy(data, data + size, &container[0]);
    }
    return container;
}

std::string DXGetBlobString(ID3DBlob* blob)
{
    return GetBlobDataTmpl<std::string>(blob);
}

std::vector<char> DXGetBlobData(ID3DBlob* blob)
{
    return GetBlobDataTmpl<std::vector<char>>(blob);
}


} // /namespace LLGL



// ================================================================================
