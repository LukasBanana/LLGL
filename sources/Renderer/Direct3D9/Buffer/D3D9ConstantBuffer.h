/*
 * D3D9ConstantBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_CONSTANT_BUFFER_H
#define LLGL_D3D9_CONSTANT_BUFFER_H


#include "D3D9Buffer.h"
#include "../Direct3D9.h"
#include <vector>


namespace LLGL
{


// D3D9 constant buffer is emulated to set multiple shader constants at once.
class D3D9ConstantBuffer final : public D3D9Buffer
{

    public:

        #include <LLGL/Backend/Buffer.inl>

    public:

        D3D9ConstantBuffer(const BufferDescriptor& desc, const void* initialData);

        // Writes new data into the buffer. This only updates the CPU data, Bind() sends it to the GPU.
        HRESULT Write(UINT dstOffset, const void* data, UINT dataSize);

        // Bind the buffer to the D3D device.
        void Bind(IDirect3DDevice9* device);

    private:

        enum D3DShaderStage
        {
            D3DShaderStage_Vertex = 0,
            D3DShaderStage_Pixel,

            D3DShaderStage_Num,
        };

        struct alignas(4) D3DConstantsHeader
        {
            UINT startRegister;
            UINT vector4Count;
        };

        struct D3DConstantsLayout
        {
            UINT firstEntry         = 0;
            UINT numEntries         = 0;

            UINT numFloatVectors    = 0;
            UINT numIntVectors      = 0;
            UINT numBoolVectors     = 0;
        };

        // Maps from interface layout to constants commands.
        struct ConstantEntry
        {
            UINT offsetInStages[D3DShaderStage_Num] = { ~0u, ~0u }; // Word offset into 'constantsCommands_'
            UINT byteSize                           = 0;
        };

    private:

        void WriteForStage(D3DShaderStage stage, UINT dstOffset, const void* data, UINT dataSize);

    private:

        std::vector<DWORD>          constantsCommands_; // Array of variable sized constants, each starting with D3DConstantsHeader
        std::vector<ConstantEntry>  entries_;
        UINT                        bufferSize_             = 0;
        D3DConstantsLayout          constantsLayouts_[D3DShaderStage_Num];

};


} // /namespace LLGL


#endif



// ================================================================================
