/*
 * D3D11ObjectUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_OBJECT_UTILS_H
#define LLGL_D3D11_OBJECT_UTILS_H


#include <d3d11.h>
#include <cstdint>
#include <string>


namespace LLGL
{


// Sets the debug name of the specified D3D device child.
void D3D11SetObjectName(ID3D11DeviceChild* obj, const char* name);

// Sets the debug name with a suffix of the specified D3D device child.
void D3D11SetObjectNameSubscript(ID3D11DeviceChild* obj, const char* name, const char* subscript);

// Sets the debug name with an index of the specified D3D device child.
void D3D11SetObjectNameIndexed(ID3D11DeviceChild* obj, const char* name, std::uint32_t index);

// Returns the debug name of the specified D3D device child.
std::string D3D11GetObjectName(ID3D11DeviceChild* obj);


} // /namespace LLGL


#endif



// ================================================================================
