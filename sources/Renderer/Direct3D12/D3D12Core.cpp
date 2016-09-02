/*
 * D3D12Core.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Core.h"
#include "../../Core/HelperMacros.h"
#include <stdexcept>


namespace LLGL
{


std::string DXErrorToStr(const HRESULT errorCode)
{
    switch (errorCode)
    {
		// see https://msdn.microsoft.com/en-us/library/windows/desktop/aa378137(v=vs.85).aspx
        LLGL_CASE_TO_STR( S_OK );
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
    }
    return "";
}

void DXThrowIfFailed(const HRESULT errorCode, const std::string& info)
{
    if (FAILED(errorCode))
        throw std::runtime_error(info + " (error code = " + DXErrorToStr(errorCode) + ")");
}


} // /namespace LLGL



// ================================================================================
