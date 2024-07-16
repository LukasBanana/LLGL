/*
 * D3D11ConstantsCache.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11ConstantsCache.h"
#include "D3D11StateManager.h"
#include "../Shader/D3D11Shader.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>
#include <string.h>


namespace LLGL
{


//TODO: move this into separate function and return error code when reflection failed
D3D11ConstantsCache::D3D11ConstantsCache(
    const ArrayView<D3D11Shader*>&      shaders,
    const ArrayView<UniformDescriptor>& uniforms)
{
    /* Reflect all constant buffers from all shaders */
    std::vector<const D3D11ConstantBufferReflection*> cbufferReflections;

    auto FindCbufferField = [&cbufferReflections](const std::string& name) -> std::pair<const D3D11ConstantBufferReflection*, const D3D11ConstantReflection*>
    {
        for (const D3D11ConstantBufferReflection* cbuffer : cbufferReflections)
        {
            for (const D3D11ConstantReflection& field : cbuffer->fields)
            {
                if (field.name == name)
                    return { cbuffer, &field };
            }
        }
        return { nullptr, nullptr };
    };

    std::uint8_t cbufferSlotMap[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
    ::memset(cbufferSlotMap, ~0, sizeof(cbufferSlotMap));

    long cbufferStageFlags[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = {};

    for (D3D11Shader* shader : shaders)
    {
        LLGL_ASSERT_PTR(shader);

        /* Get cached cbuffer reflection from shader */
        const std::vector<D3D11ConstantBufferReflection>* currentCbufferReflections = nullptr;
        HRESULT hr = shader->ReflectAndCacheConstantBuffers(&currentCbufferReflections);
        DXThrowIfFailed(hr, "failed to reflect constant buffers in D3D11 shader");
        LLGL_ASSERT_PTR(currentCbufferReflections);

        cbufferReflections.reserve(cbufferReflections.size() + currentCbufferReflections->size());
        for (const D3D11ConstantBufferReflection& cbufferReflection : *currentCbufferReflections)
        {
            cbufferReflections.push_back(&cbufferReflection);
            LLGL_ASSERT(cbufferReflection.slot < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
            cbufferStageFlags[cbufferReflection.slot] |= GetStageFlags(shader->GetType());
        }
    }

    /* Create root signature copy and append parameters to permutation */
    constantsMap_.resize(uniforms.size());

    for_range(i, uniforms.size())
    {
        /* Find constant buffer field for specified uniform name */
        auto field = FindCbufferField(uniforms[i].name);
        const auto* cbufferReflection = field.first;
        const auto* fieldReflection = field.second;

        if (cbufferReflection == nullptr || fieldReflection == nullptr)
            continue;

        /* Allocate cache for constant buffer and assgin index to cbuffer-slot map */
        std::uint8_t& cbufferIndex = cbufferSlotMap[cbufferReflection->slot];
        if (cbufferIndex == 0xFF)
        {
            cbufferIndex = AllocateConstantBuffer(
                cbufferReflection->slot,
                cbufferReflection->size,
                cbufferStageFlags[cbufferReflection->slot]
            );
        }

        /* Build root constant map for current uniform descriptor */
        D3D11ConstantsCache::ConstantLocation& location = constantsMap_[i];
        {
            location.index  = cbufferIndex;
            location.size   = fieldReflection->size;
            location.offset = fieldReflection->offset;
        }
    }

    /* Allocate bit-vector for cbuffer invalidation states */
    invalidatedBuffers_.resize(constantBuffers_.size());
}

HRESULT D3D11ConstantsCache::SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
{
    for (auto* dataByteAligned = reinterpret_cast<const char*>(data); dataSize > 0; ++first)
    {
        if (first >= constantsMap_.size())
            return E_INVALIDARG;

        /* Get current uniform location with its cbuffer */
        const D3D11ConstantsCache::ConstantLocation& location = constantsMap_[first];
        D3D11ConstantsCache::ConstantBuffer& cbuffer = constantBuffers_[location.index];
        const std::uint16_t chunkSize = std::min<std::uint16_t>(dataSize, static_cast<std::uint16_t>(location.size));

        /* Copy input data into cbuffer data and move to next uniform */
        ::memcpy(reinterpret_cast<char*>(cbuffer.constants.data()) + location.offset, dataByteAligned, chunkSize);

        dataByteAligned += chunkSize;
        dataSize -= chunkSize;

        /* Invalidate cache for current cbuffer */
        if (!invalidatedBuffers_[location.index])
        {
            invalidatedBuffers_[location.index] = true;
            invalidatedBuffersRange_[0] = std::min<std::uint8_t>(invalidatedBuffersRange_[0], location.index);
            invalidatedBuffersRange_[1] = std::max<std::uint8_t>(invalidatedBuffersRange_[1], location.index + 1u);
        }
    }
    return S_OK;
}

void D3D11ConstantsCache::Reset()
{
    /* Reset range to special value to indicate that all cbuffers have to be bound again */
    invalidatedBuffersRange_[0] = 0x00;
    invalidatedBuffersRange_[1] = 0xFF;
}

void D3D11ConstantsCache::Flush(D3D11StateManager& stateMngr)
{
    if (invalidatedBuffersRange_[0] < invalidatedBuffersRange_[1])
    {
        /* Check for special range to indicate that all cbuffers have to be bound again */
        if (invalidatedBuffersRange_[0] == 0x00 &&
            invalidatedBuffersRange_[1] == 0xFF)
        {
            for_range(i, static_cast<std::uint8_t>(constantBuffers_.size()))
                FlushConstantBuffer(i, stateMngr);
        }
        else
        {
            for_subrange(i, invalidatedBuffersRange_[0], invalidatedBuffersRange_[1])
            {
                if (invalidatedBuffers_[i])
                    FlushConstantBuffer(i, stateMngr);
            }
        }

        /* Reset constant buffer pool; We only need unique staging buffers for each cbuffer in this cache before the next draw call */
        stateMngr.ResetCbufferPool();

        /* Clear cached range */
        invalidatedBuffersRange_[0] = 0xFF;
        invalidatedBuffersRange_[1] = 0x00;
    }
}


/*
 * ======= Private: =======
 */

std::uint8_t D3D11ConstantsCache::AllocateConstantBuffer(UINT slot, UINT size, long stageFlags)
{
    const std::size_t nextIndex = constantBuffers_.size();
    const UINT numConstants = DivideRoundUp<UINT>(size, sizeof(ConstantRegister));
    constantBuffers_.resize(nextIndex + 1);
    auto& cbuffer = constantBuffers_.back();
    {
        cbuffer.shaderRegister  = slot;
        cbuffer.stageFlags      = stageFlags;
        cbuffer.constants.resize(numConstants);
    }
    return static_cast<std::uint8_t>(nextIndex);
}

void D3D11ConstantsCache::FlushConstantBuffer(std::uint8_t index, D3D11StateManager& stateMngr)
{
    invalidatedBuffers_[index] = false;
    const ConstantBuffer& cbuffer = constantBuffers_[index];
    stateMngr.SetConstants(
        cbuffer.shaderRegister,
        cbuffer.constants.data(),
        static_cast<UINT>(cbuffer.constants.size() * sizeof(ConstantRegister)),
        cbuffer.stageFlags
    );
}


} // /namespace LLGL



// ================================================================================
