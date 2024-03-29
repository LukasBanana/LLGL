/*
 * D3D11ObjectUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <initguid.h> // Comes first to define GUIDs
#include "D3D11ObjectUtils.h"
#include <string>
#include <cstring>


namespace LLGL
{


void D3D11SetObjectName(ID3D11DeviceChild* obj, const char* name)
{
    if (obj != nullptr)
    {
        if (name != nullptr)
        {
            const std::size_t nameLen = std::strlen(name);
            obj->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(nameLen), name);
        }
        else
            obj->SetPrivateData(WKPDID_D3DDebugObjectName, 0, nullptr);
    }
}

void D3D11SetObjectNameSubscript(ID3D11DeviceChild* obj, const char* name, const char* subscript)
{
    if (obj != nullptr)
    {
        if (name != nullptr)
        {
            std::string nameWithSubscript = name;
            nameWithSubscript += subscript;
            const std::size_t nameLen = nameWithSubscript.size();
            obj->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(nameLen), nameWithSubscript.c_str());
        }
        else
            obj->SetPrivateData(WKPDID_D3DDebugObjectName, 0, nullptr);
    }
}

void D3D11SetObjectNameIndexed(ID3D11DeviceChild* obj, const char* name, std::uint32_t index)
{
    if (name != nullptr)
    {
        /* Append subscript to label */
        const std::string subscript = std::to_string(index);
        D3D11SetObjectNameSubscript(obj, name, subscript.c_str());
    }
    else
        D3D11SetObjectName(obj, nullptr);
}

std::string D3D11GetObjectName(ID3D11DeviceChild* obj)
{
    if (obj != nullptr)
    {
        UINT nameLen = 0;
        obj->GetPrivateData(WKPDID_D3DDebugObjectName, &nameLen, nullptr);
        std::string name;
        name.resize(nameLen);
        obj->GetPrivateData(WKPDID_D3DDebugObjectName, &nameLen, &name[0]);
        return name;
    }
    return "";
}


} // /namespace LLGL



// ================================================================================
