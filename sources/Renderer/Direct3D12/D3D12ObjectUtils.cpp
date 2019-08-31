/*
 * D3D12ObjectUtils.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12ObjectUtils.h"
#include <string>
#include <cstring>
#include <codecvt>


namespace LLGL
{


static void D3D12SetObjectNameUTF8toUTF16(ID3D12Object* obj, const char* name)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring nameWStr = converter.from_bytes(name);
    obj->SetName(nameWStr.c_str());
}

void D3D12SetObjectName(ID3D12Object* obj, const char* name)
{
    if (obj != nullptr)
    {
        if (name != nullptr)
            D3D12SetObjectNameUTF8toUTF16(obj, name);
        else
            obj->SetName(nullptr);
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
            D3D12SetObjectNameUTF8toUTF16(obj, nameWithSubscript.c_str());
        }
        else
            obj->SetName(nullptr);
    }
}

void D3D12SetObjectNameIndexed(ID3D12Object* obj, const char* name, std::uint32_t index)
{
    if (name != nullptr)
    {
        /* Append subscript to label */
        std::string subscript = std::to_string(index);
        D3D12SetObjectNameSubscript(obj, name, subscript.c_str());
    }
    else
        obj->SetName(nullptr);
}


} // /namespace LLGL



// ================================================================================
