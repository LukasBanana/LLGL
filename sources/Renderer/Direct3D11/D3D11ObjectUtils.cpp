/*
 * D3D11ObjectUtils.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11ObjectUtils.h"
#include <string>
#include <cstring>


namespace LLGL
{


// Declare custom object of "WKPDID_D3DDebugObjectName" as defined in <d3dcommon.h> to avoid linking with "dxguid.lib"
static const GUID g_WKPDID_D3DDebugObjectName = { 0x429b8c22, 0x9188, 0x4b0c, { 0x87,0x42,0xac,0xb0,0xbf,0x85,0xc2,0x00 } };

void D3D11SetObjectName(ID3D11DeviceChild* obj, const char* name)
{
    if (obj != nullptr)
    {
        if (name != nullptr)
        {
            const std::size_t length = std::strlen(name);
            obj->SetPrivateData(g_WKPDID_D3DDebugObjectName, static_cast<UINT>(length), name);
        }
        else
            obj->SetPrivateData(g_WKPDID_D3DDebugObjectName, 0, nullptr);
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
            const std::size_t length = nameWithSubscript.size();
            obj->SetPrivateData(g_WKPDID_D3DDebugObjectName, static_cast<UINT>(length), nameWithSubscript.c_str());
        }
        else
            obj->SetPrivateData(g_WKPDID_D3DDebugObjectName, 0, nullptr);
    }
}

void D3D11SetObjectNameIndexed(ID3D11DeviceChild* obj, const char* name, std::uint32_t index)
{
    if (name != nullptr)
    {
        /* Append subscript to label */
        std::string subscript = std::to_string(index);
        D3D11SetObjectNameSubscript(obj, name, subscript.c_str());
    }
    else
        D3D11SetObjectName(obj, nullptr);
}


} // /namespace LLGL



// ================================================================================
