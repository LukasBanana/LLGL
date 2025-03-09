/*
 * D3D12ObjectUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12ObjectUtils.h"
#include "../DXCommon/DXCore.h"
#include <string>
#include <cstring>


namespace LLGL
{


void D3D12SetObjectName(ID3D12Object* obj, const char* name)
{
    if (obj != nullptr)
    {
        if (name != nullptr)
        {
            const std::size_t nameLen = std::strlen(name);
            obj->SetPrivateData(DXGetD3DDebugObjectNameGUID(), static_cast<UINT>(nameLen), name);
        }
        else
            obj->SetPrivateData(DXGetD3DDebugObjectNameGUID(), 0, nullptr);
    }
}

void D3D12SetObjectNameSubscript(ID3D12Object* obj, const char* name, const char* subscript)
{
    if (obj != nullptr)
    {
        if (name != nullptr)
        {
            std::string nameWithSubscript = name;
            nameWithSubscript += subscript;
            const std::size_t nameLen = nameWithSubscript.size();
            obj->SetPrivateData(DXGetD3DDebugObjectNameGUID(), static_cast<UINT>(nameLen), nameWithSubscript.c_str());
        }
        else
            obj->SetPrivateData(DXGetD3DDebugObjectNameGUID(), 0, nullptr);
    }
}

void D3D12SetObjectNameIndexed(ID3D12Object* obj, const char* name, std::uint32_t index)
{
    if (name != nullptr)
    {
        /* Append subscript to label */
        const std::string subscript = std::to_string(index);
        D3D12SetObjectNameSubscript(obj, name, subscript.c_str());
    }
    else
        D3D12SetObjectName(obj, nullptr);
}

std::string D3D12GetObjectName(ID3D12Object* obj)
{
    if (obj != nullptr)
    {
        UINT nameLen = 0;
        obj->GetPrivateData(DXGetD3DDebugObjectNameGUID(), &nameLen, nullptr);
        std::string name;
        name.resize(nameLen);
        obj->GetPrivateData(DXGetD3DDebugObjectNameGUID(), &nameLen, &name[0]);
        return name;
    }
    return "";
}


} // /namespace LLGL



// ================================================================================
