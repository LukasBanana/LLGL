/*
 * D3D12ObjectUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_OBJECT_UTILS_H
#define LLGL_D3D12_OBJECT_UTILS_H


#include <d3d12.h>
#include <cstdint>
#include <string>


namespace LLGL
{


// Sets the debug name of the specified D3D device child.
void D3D12SetObjectName(ID3D12Object* obj, const char* name);

// Sets the debug name with a suffix of the specified D3D device child.
void D3D12SetObjectNameSubscript(ID3D12Object* obj, const char* name, const char* subscript);

// Sets the debug name with an index of the specified D3D device child.
void D3D12SetObjectNameIndexed(ID3D12Object* obj, const char* name, std::uint32_t index);

// Returns the debug name of the specified D3D device child.
std::string D3D12GetObjectName(ID3D12Object* obj);


} // /namespace LLGL


#endif



// ================================================================================
