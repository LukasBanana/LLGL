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
