/*
 * D3D9ConstantBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9ConstantBuffer.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <string.h>
#include <algorithm>


namespace LLGL
{


D3D9ConstantBuffer::D3D9ConstantBuffer(const BufferDescriptor& desc, const void* initialData) :
    D3D9Buffer { desc.bindFlags }
{
    if (initialData != nullptr)
        Write(0, initialData, static_cast<UINT>(desc.size));
}

BufferDescriptor D3D9ConstantBuffer::GetDesc() const
{
    BufferDescriptor desc;
    {
        desc.bindFlags  = BindFlags::ConstantBuffer;
        desc.size       = bufferSize_;
    }
    return desc;
}

bool D3D9ConstantBuffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    return false; // dummy
}

HRESULT D3D9ConstantBuffer::Write(UINT dstOffset, const void* data, UINT dataSize)
{
    const UINT dstOffsetEnd = dstOffset + dataSize;
    if (dstOffsetEnd < dstOffset || dstOffsetEnd > bufferSize_)
        return E_BOUNDS;

    WriteForStage(D3DShaderStage_Vertex, dstOffset, data, dataSize);
    WriteForStage(D3DShaderStage_Pixel,  dstOffset, data, dataSize);

    return S_OK;
}

#define SET_D3D9_SHADER_CONSTANTS(FUNC, TYPE, COUNT)                                                    \
    for (UINT numVectors = (COUNT); numVectors > 0;)                                                    \
    {                                                                                                   \
        auto* header = reinterpret_cast<const D3DConstantsHeader*>(commands);                           \
        LLGL_ASSERT(numVectors >= header->vector4Count);                                                \
        FUNC(header->startRegister, reinterpret_cast<const TYPE*>(header + 1), header->vector4Count);   \
        numVectors -= header->vector4Count;                                                             \
        commands += (sizeof(D3DConstantsHeader)/sizeof(DWORD) + header->vector4Count*4);                \
    }

void D3D9ConstantBuffer::Bind(IDirect3DDevice9* device)
{
    const DWORD* commands = constantsCommands_.data();

    const D3DConstantsLayout& vertexLayout = constantsLayouts_[D3DShaderStage_Vertex];
    if (vertexLayout.numEntries > 0)
    {
        SET_D3D9_SHADER_CONSTANTS(device->SetVertexShaderConstantF, FLOAT,  vertexLayout.numFloatVectors);
        SET_D3D9_SHADER_CONSTANTS(device->SetVertexShaderConstantI, INT,    vertexLayout.numIntVectors  );
        SET_D3D9_SHADER_CONSTANTS(device->SetVertexShaderConstantB, BOOL,   vertexLayout.numBoolVectors );
    }

    const D3DConstantsLayout& pixelLayout = constantsLayouts_[D3DShaderStage_Pixel];
    if (pixelLayout.numEntries > 0)
    {
        SET_D3D9_SHADER_CONSTANTS(device->SetPixelShaderConstantF,  FLOAT,  pixelLayout.numFloatVectors );
        SET_D3D9_SHADER_CONSTANTS(device->SetPixelShaderConstantI,  INT,    pixelLayout.numIntVectors   );
        SET_D3D9_SHADER_CONSTANTS(device->SetPixelShaderConstantB,  BOOL,   pixelLayout.numBoolVectors  );
    }
}

#undef SET_D3D9_SHADER_CONSTANTS


/*
 * ======= Private: =======
 */

void D3D9ConstantBuffer::WriteForStage(D3DShaderStage stage, UINT dstOffset, const void* data, UINT dataSize)
{
    /* No need to search or write anything if there are no entires for this stage */
    const D3DConstantsLayout& layout = constantsLayouts_[stage];
    if (layout.numFloatVectors == 0 &&
        layout.numIntVectors   == 0 &&
        layout.numBoolVectors  == 0)
    {
        return;
    }

    /* Find first entry index with binary search from destination offest */
    std::size_t entryIndex = layout.firstEntry;

    while (dstOffset > 0)
    {
        const UINT entrySize = std::min<UINT>(dstOffset, entries_[entryIndex].byteSize);
        dstOffset -= entrySize;
        ++entryIndex;
    }

    /* Write to each entry until data is fully written */
    for (const char* byteData = static_cast<const char*>(data); dataSize > 0; ++entryIndex)
    {
        const ConstantEntry& entry = entries_[entryIndex];
        const UINT offsetInStage = entry.offsetInStages[stage];

        DWORD* dst = &(constantsCommands_[offsetInStage]);

        const UINT entrySize = std::min<UINT>(dataSize, entry.byteSize);
        ::memcpy(dst, byteData, entrySize);

        dataSize -= entrySize;
        byteData += entrySize;
    }
}


} // /namespace LLGL



// ================================================================================
