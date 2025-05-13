/*
 * D3D11ObjectUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11ObjectUtils.h"
#include "../DXCommon/DXCore.h"
#include <LLGL/STL/String.h>
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
            obj->SetPrivateData(DXGetD3DDebugObjectNameGUID(), static_cast<UINT>(nameLen), name);
        }
        else
            obj->SetPrivateData(DXGetD3DDebugObjectNameGUID(), 0, nullptr);
    }
}

void D3D11SetObjectNameSubscript(ID3D11DeviceChild* obj, const char* name, const char* subscript)
{
    if (obj != nullptr)
    {
        if (name != nullptr)
        {
            STL::string nameWithSubscript = name;
            nameWithSubscript += subscript;
            const std::size_t nameLen = nameWithSubscript.size();
            obj->SetPrivateData(DXGetD3DDebugObjectNameGUID(), static_cast<UINT>(nameLen), nameWithSubscript.c_str());
        }
        else
            obj->SetPrivateData(DXGetD3DDebugObjectNameGUID(), 0, nullptr);
    }
}

void D3D11SetObjectNameIndexed(ID3D11DeviceChild* obj, const char* name, std::uint32_t index)
{
    if (name != nullptr)
    {
        /* Append subscript to label */
        const STL::string subscript = std::to_string(index);
        D3D11SetObjectNameSubscript(obj, name, subscript.c_str());
    }
    else
        D3D11SetObjectName(obj, nullptr);
}

STL::string D3D11GetObjectName(ID3D11DeviceChild* obj)
{
    if (obj != nullptr)
    {
        UINT nameLen = 0;
        obj->GetPrivateData(DXGetD3DDebugObjectNameGUID(), &nameLen, nullptr);
        STL::string name;
        name.resize(nameLen);
        obj->GetPrivateData(DXGetD3DDebugObjectNameGUID(), &nameLen, &name[0]);
        return name;
    }
    return "";
}

void D3D11ThrowIfFailed(HRESULT hr, const char* info, ID3D11DeviceChild* obj)
{
    if (FAILED(hr))
    {
        if (obj != nullptr)
        {
            const STL::string infoExt = STL::string(info) + " \"" + D3D11GetObjectName(obj) + "\"";
            DXThrowIfFailed(hr, infoExt.c_str());
        }
        else
            DXThrowIfFailed(hr, info);
    }
}


} // /namespace LLGL



// ================================================================================
