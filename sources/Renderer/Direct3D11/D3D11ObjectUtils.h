/*
 * D3D11ObjectUtils.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_OBJECT_UTILS_H
#define LLGL_D3D11_OBJECT_UTILS_H


#include <d3d11.h>
#include <cstdint>


namespace LLGL
{


// Sets the debug name of the specified D3D device child.
void D3D11SetObjectName(ID3D11DeviceChild* obj, const char* name);

// Sets the debug name with a suffix of the specified D3D device child.
void D3D11SetObjectNameSubscript(ID3D11DeviceChild* obj, const char* name, const char* subscript);

// Sets the debug name with an index of the specified D3D device child.
void D3D11SetObjectNameIndexed(ID3D11DeviceChild* obj, const char* name, std::uint32_t index);


} // /namespace LLGL


#endif



// ================================================================================
