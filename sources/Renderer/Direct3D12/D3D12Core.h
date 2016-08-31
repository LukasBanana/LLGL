/*
 * D3D12Core.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D12_CORE_H__
#define __LLGL_D3D12_CORE_H__


#include <string>
#include <Windows.h>


namespace LLGL
{


//! Release the specified D3D12 object.
template <typename T>
void SafeRelease(T*& obj)
{
    if (obj)
    {
        obj->Release();
        obj = nullptr;
    }
}


//! Converts the DX error code into a string.
std::string DXErrorToStr(const HRESULT errorCode);

//! Throws an std::runtime_error exception of 'errorCode' is not S_OK.
void DXThrowIfFailed(const HRESULT errorCode, const std::string& info);


} // /namespace LLGL


#endif



// ================================================================================
