/*
 * D3D12ObjectUtils.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_OBJECT_UTILS_H
#define LLGL_D3D12_OBJECT_UTILS_H


#include <d3d12.h>
#include <cstdint>


namespace LLGL
{


// Sets the debug name of the specified D3D device child.
void D3D12SetObjectName(ID3D12Object* obj, const char* name);

// Sets the debug name with a suffix of the specified D3D device child.
void D3D12SetObjectNameSubscript(ID3D12Object* obj, const char* name, const char* subscript);

// Sets the debug name with an index of the specified D3D device child.
void D3D12SetObjectNameIndexed(ID3D12Object* obj, const char* name, std::uint32_t index);


} // /namespace LLGL


#endif



// ================================================================================
