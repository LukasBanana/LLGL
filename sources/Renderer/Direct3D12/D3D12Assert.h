/*
 * D3D12Assert.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D12_ASSERT_H__
#define __LLGL_D3D12_ASSERT_H__


#include <string>
#include <stdexcept>
#include <d3d12.h>


namespace LLGL
{


#ifdef LLGL_DEBUG

template <typename T>
T* D3D12Assert(T* obj, const char* procName, const char* typeName)
{
    if (!obj)
    {
        std::string error = std::string(procName) + ": null pointer exception of D3D12 object";
        if (typeName)
            error += " \"" + std::string(typeName) + "\"";
        throw std::runtime_error(error);
    }
    return obj;
}

template <typename T>
const char* D3D12TypeName(T*)
{
    return nullptr;
}

template <>
const char* D3D12TypeName(ID3D12GraphicsCommandList*)
{
    return "ID3D12GraphicsCommandList";
}

#define LLGL_D3D_ASSERT(OBJ) D3D12Assert(OBJ, __FUNCTION__, D3D12TypeName(OBJ))

#else

#define LLGL_D3D_ASSERT(OBJ) (OBJ)

#endif


} // /namespace LLGL


#endif



// ================================================================================
