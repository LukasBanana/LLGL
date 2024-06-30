/*
 * D3D12MipGenerator.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12MipGenerator.h"
#include "D3D12Texture.h"
#include "../Shader/Builtin/D3D12Builtin.h"
#include "../Command/D3D12CommandContext.h"
#include "../../DXCommon/DXCore.h"
#include "../../DXCommon/DXTypes.h"
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


D3D12MipGenerator& D3D12MipGenerator::Get()
{
    static D3D12MipGenerator instance;
    return instance;
}

void D3D12MipGenerator::InitializeDevice(ID3D12Device* device)
{
    /* Store device object and GPU descriptor handle size */
    device_         = device;
    descHandleSize_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    /* Create resources for 1D, 2D, and 3D MIP-map generation */
    CreateResourcesFor1DMips(device);
    CreateResourcesFor2DMips(device);
    CreateResourcesFor3DMips(device);
}

template <std::size_t N>
void ReleasePipelinesAndRootSignature(
    ComPtr<ID3D12RootSignature>& rootSignature,
    ComPtr<ID3D12PipelineState> (&pipelineStates)[N])
{
    for (auto& pso : pipelineStates)
        pso.Reset();
    rootSignature.Reset();
}

void D3D12MipGenerator::Clear()
{
    ReleasePipelinesAndRootSignature(rootSignature1D_, pipelines1D_);
    ReleasePipelinesAndRootSignature(rootSignature2D_, pipelines2D_);
    ReleasePipelinesAndRootSignature(rootSignature3D_, pipelines3D_);
}

HRESULT D3D12MipGenerator::GenerateMips(
    D3D12CommandContext&        commandContext,
    D3D12Texture&               texture,
    const TextureSubresource&   subresource)
{
    if (!texture.SupportsGenerateMips())
    {
        /* Texture does not support generation of MIP-maps */
        return E_INVALIDARG;
    }

    if (subresource.numMipLevels <= 1 || subresource.numArrayLayers == 0)
    {
        /* Ignore this call, no MIP-map range specified */
        return S_OK;
    }

    if (subresource.baseMipLevel + subresource.numMipLevels > texture.GetNumMipLevels() ||
        subresource.baseArrayLayer + subresource.numArrayLayers > texture.GetNumArrayLayers())
    {
        /* Invalid subresource MIP-map level or array layer range */
        return E_INVALIDARG;
    }

    ID3D12DescriptorHeap* mipDescHeap = texture.GetMipDescHeap();
    if (mipDescHeap == nullptr)
    {
        /* At this point, the texture should have a valid descriptor heap */
        return E_FAIL;
    }

    switch (texture.GetType())
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            GenerateMips1D(commandContext, texture, mipDescHeap, subresource);
            return S_OK;

        case TextureType::Texture2D:
        case TextureType::TextureCube:
        case TextureType::Texture2DArray:
        case TextureType::TextureCubeArray:
            GenerateMips2D(commandContext, texture, mipDescHeap, subresource);
            return S_OK;

        case TextureType::Texture3D:
            GenerateMips3D(commandContext, texture, mipDescHeap, subresource);
            return S_OK;

        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            // no MIP-maps for multi-sampled textures
            break;
    }

    /* Unknown argument or corrupted data */
    return E_FAIL;
}


/*
 * ======= Private: =======
 */

ComPtr<ID3D12PipelineState> D3D12MipGenerator::CreateComputePSO(
    ID3D12Device*           device,
    ID3D12RootSignature*    rootSignature,
    const BYTE*             shaderBytecode,
    size_t                  shaderBytecodeSize)
{
    ComPtr<ID3D12PipelineState> pipelineState;

    if (ComPtr<ID3DBlob> blob = DXCreateBlob(shaderBytecode, shaderBytecodeSize))
    {
        /* Create graphics pipeline state and graphics command list */
        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
        {
            psoDesc.pRootSignature      = rootSignature;
            psoDesc.CS.pShaderBytecode  = blob->GetBufferPointer();
            psoDesc.CS.BytecodeLength   = blob->GetBufferSize();
        }
        HRESULT hr = device_->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));
        DXThrowIfCreateFailed(hr, "ID3D12PipelineState");
    }

    return pipelineState;
}

void D3D12MipGenerator::CreateResourcesFor1DMips(ID3D12Device* device)
{
    /* Initialize root signature */
    D3D12RootSignature rootSignature;
    {
        rootSignature.ResetAndAlloc(3, 1);
        rootSignature[0].InitAsConstants(0, 4);
        rootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
        rootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 8);
        auto samplerDesc = rootSignature.AppendStaticSampler();
        {
            samplerDesc->Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        }
    }
    rootSignature1D_ = rootSignature.Finalize(device);

    /* Create pre-build PSOs */
    pipelines1D_[0x0] = CreateComputePSO( device, rootSignature1D_.Get(), LLGL_IDR_GENERATEMIPS1D_CS,           sizeof(LLGL_IDR_GENERATEMIPS1D_CS)          );
    pipelines1D_[0x1] = CreateComputePSO( device, rootSignature1D_.Get(), LLGL_IDR_GENERATEMIPS1D_CS_ODDX,      sizeof(LLGL_IDR_GENERATEMIPS1D_CS_ODDX)     );
    pipelines1D_[0x2] = CreateComputePSO( device, rootSignature1D_.Get(), LLGL_IDR_GENERATEMIPS1D_CS_SRGB,      sizeof(LLGL_IDR_GENERATEMIPS1D_CS_SRGB)     );
    pipelines1D_[0x3] = CreateComputePSO( device, rootSignature1D_.Get(), LLGL_IDR_GENERATEMIPS1D_CS_SRGB_ODDX, sizeof(LLGL_IDR_GENERATEMIPS1D_CS_SRGB_ODDX));
}

void D3D12MipGenerator::CreateResourcesFor2DMips(ID3D12Device* device)
{
    /* Initialize root signature */
    D3D12RootSignature rootSignature;
    {
        rootSignature.ResetAndAlloc(3, 1);
        rootSignature[0].InitAsConstants(0, 5);
        rootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
        rootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 4);
        auto samplerDesc = rootSignature.AppendStaticSampler();
        {
            samplerDesc->Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        }
    }
    rootSignature2D_ = rootSignature.Finalize(device);

    /* Create pre-build PSOs */
    pipelines2D_[0x0] = CreateComputePSO( device, rootSignature2D_.Get(), LLGL_IDR_GENERATEMIPS2D_CS,             sizeof(LLGL_IDR_GENERATEMIPS2D_CS)            );
    pipelines2D_[0x1] = CreateComputePSO( device, rootSignature2D_.Get(), LLGL_IDR_GENERATEMIPS2D_CS_ODDX,        sizeof(LLGL_IDR_GENERATEMIPS2D_CS_ODDX)       );
    pipelines2D_[0x2] = CreateComputePSO( device, rootSignature2D_.Get(), LLGL_IDR_GENERATEMIPS2D_CS_ODDY,        sizeof(LLGL_IDR_GENERATEMIPS2D_CS_ODDY)       );
    pipelines2D_[0x3] = CreateComputePSO( device, rootSignature2D_.Get(), LLGL_IDR_GENERATEMIPS2D_CS_ODDXY,       sizeof(LLGL_IDR_GENERATEMIPS2D_CS_ODDXY)      );
    pipelines2D_[0x4] = CreateComputePSO( device, rootSignature2D_.Get(), LLGL_IDR_GENERATEMIPS2D_CS_SRGB,        sizeof(LLGL_IDR_GENERATEMIPS2D_CS_SRGB)       );
    pipelines2D_[0x5] = CreateComputePSO( device, rootSignature2D_.Get(), LLGL_IDR_GENERATEMIPS2D_CS_SRGB_ODDX,   sizeof(LLGL_IDR_GENERATEMIPS2D_CS_SRGB_ODDX)  );
    pipelines2D_[0x6] = CreateComputePSO( device, rootSignature2D_.Get(), LLGL_IDR_GENERATEMIPS2D_CS_SRGB_ODDY,   sizeof(LLGL_IDR_GENERATEMIPS2D_CS_SRGB_ODDY)  );
    pipelines2D_[0x7] = CreateComputePSO( device, rootSignature2D_.Get(), LLGL_IDR_GENERATEMIPS2D_CS_SRGB_ODDXY,  sizeof(LLGL_IDR_GENERATEMIPS2D_CS_SRGB_ODDXY) );
}

void D3D12MipGenerator::CreateResourcesFor3DMips(ID3D12Device* device)
{
    /* Initialize root signature */
    D3D12RootSignature rootSignature;
    {
        rootSignature.ResetAndAlloc(3, 1);
        rootSignature[0].InitAsConstants(0, 5);
        rootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
        rootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 3);
        rootSignature.AppendStaticSampler();
    }
    rootSignature3D_ = rootSignature.Finalize(device);

    /* Create pre-build PSOs */
    pipelines3D_[0x0] = CreateComputePSO( device, rootSignature3D_.Get(), LLGL_IDR_GENERATEMIPS3D_CS,             sizeof(LLGL_IDR_GENERATEMIPS3D_CS)            );
    pipelines3D_[0x1] = CreateComputePSO( device, rootSignature3D_.Get(), LLGL_IDR_GENERATEMIPS3D_CS_ODDX,        sizeof(LLGL_IDR_GENERATEMIPS3D_CS_ODDX)       );
    pipelines3D_[0x2] = CreateComputePSO( device, rootSignature3D_.Get(), LLGL_IDR_GENERATEMIPS3D_CS_ODDY,        sizeof(LLGL_IDR_GENERATEMIPS3D_CS_ODDY)       );
    pipelines3D_[0x3] = CreateComputePSO( device, rootSignature3D_.Get(), LLGL_IDR_GENERATEMIPS3D_CS_ODDXY,       sizeof(LLGL_IDR_GENERATEMIPS3D_CS_ODDXY)      );
    pipelines3D_[0x4] = CreateComputePSO( device, rootSignature3D_.Get(), LLGL_IDR_GENERATEMIPS3D_CS_ODDZ,        sizeof(LLGL_IDR_GENERATEMIPS3D_CS_ODDZ)       );
    pipelines3D_[0x5] = CreateComputePSO( device, rootSignature3D_.Get(), LLGL_IDR_GENERATEMIPS3D_CS_ODDXZ,       sizeof(LLGL_IDR_GENERATEMIPS3D_CS_ODDXZ)      );
    pipelines3D_[0x6] = CreateComputePSO( device, rootSignature3D_.Get(), LLGL_IDR_GENERATEMIPS3D_CS_ODDYZ,       sizeof(LLGL_IDR_GENERATEMIPS3D_CS_ODDYZ)      );
    pipelines3D_[0x7] = CreateComputePSO( device, rootSignature3D_.Get(), LLGL_IDR_GENERATEMIPS3D_CS_ODDXYZ,      sizeof(LLGL_IDR_GENERATEMIPS3D_CS_ODDXYZ)     );
    pipelines3D_[0x8] = CreateComputePSO( device, rootSignature3D_.Get(), LLGL_IDR_GENERATEMIPS3D_CS_SRGB,        sizeof(LLGL_IDR_GENERATEMIPS3D_CS_SRGB)       );
    pipelines3D_[0x9] = CreateComputePSO( device, rootSignature3D_.Get(), LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDX,   sizeof(LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDX)  );
    pipelines3D_[0xA] = CreateComputePSO( device, rootSignature3D_.Get(), LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDY,   sizeof(LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDY)  );
    pipelines3D_[0xB] = CreateComputePSO( device, rootSignature3D_.Get(), LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDXY,  sizeof(LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDXY) );
    pipelines3D_[0xC] = CreateComputePSO( device, rootSignature3D_.Get(), LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDZ,   sizeof(LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDZ)  );
    pipelines3D_[0xD] = CreateComputePSO( device, rootSignature3D_.Get(), LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDXZ,  sizeof(LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDXZ) );
    pipelines3D_[0xE] = CreateComputePSO( device, rootSignature3D_.Get(), LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDYZ,  sizeof(LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDYZ) );
    pipelines3D_[0xF] = CreateComputePSO( device, rootSignature3D_.Get(), LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDXYZ, sizeof(LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDXYZ));
}

void D3D12MipGenerator::GenerateMips1D(
    D3D12CommandContext&        commandContext,
    D3D12Texture&               texture,
    ID3D12DescriptorHeap*       mipDescHeap,
    const TextureSubresource&   subresource)
{
    const bool isFormatSRGB = DXTypes::IsDXGIFormatSRGB(texture.GetDXFormat());

    ID3D12GraphicsCommandList* commandList = commandContext.GetCommandList();
    D3D12Resource& resource = texture.GetResource();

    const D3D12_RESOURCE_STATES oldResourceState = texture.GetResource().currentState;
    commandContext.TransitionResource(resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

    /* Set root signature and descriptor heap */
    commandContext.SetComputeRootSignature(rootSignature1D_.Get());

    ID3D12DescriptorHeap* descHeaps[] = { mipDescHeap };
    commandContext.SetDescriptorHeaps(1, descHeaps);

    D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle = mipDescHeap->GetGPUDescriptorHandleForHeapStart();

    /* Set SRV to read from entire MIP-map chain */
    commandList->SetComputeRootDescriptorTable(1, gpuDescHandle);
    gpuDescHandle.ptr += descHandleSize_;

    D3D12_RESOURCE_DESC resourceDesc = texture.GetNative()->GetDesc();

    const std::uint32_t mipLevelEnd = subresource.baseMipLevel + subresource.numMipLevels - 1;

    for (std::uint32_t mipLevel = subresource.baseMipLevel; mipLevel < mipLevelEnd;)
    {
        /* Determine source and destination extents */
        UINT srcWidth = static_cast<UINT>(resourceDesc.Width) >> mipLevel;

        UINT dstWidth = std::max(1u, srcWidth >> 1);

        /* Bind pipeline state depending on power-of-two class */
        UINT nonPowerOfTwo = (srcWidth & 1);
        if (isFormatSRGB)
            commandContext.SetPipelineState(pipelines1D_[nonPowerOfTwo + 2].Get());
        else
            commandContext.SetPipelineState(pipelines1D_[nonPowerOfTwo].Get());

        /* Determine how many MIP-maps can be downsampled at once; must be in [1, 8] */
        UINT numMips = (mipLevel + 8 >= mipLevelEnd ? mipLevelEnd - mipLevel : 8);

        /* Run compute shader to generate next four MIP-maps */
        commandContext.SetComputeConstant(0, 1.0f / static_cast<float>(dstWidth), 0);
        commandContext.SetComputeConstant(0, mipLevel, 1);
        commandContext.SetComputeConstant(0, numMips, 2);
        commandContext.SetComputeConstant(0, subresource.baseArrayLayer, 3);

        commandList->SetComputeRootDescriptorTable(2, gpuDescHandle);
        gpuDescHandle.ptr += descHandleSize_ * numMips;

        commandList->Dispatch(
            DivideRoundUp(dstWidth, 64u),
            subresource.numArrayLayers,
            1u
        );

        /* Insert UAV barrier and move to next four MIP-maps */
        commandContext.UAVBarrier(texture.GetNative(), true);

        mipLevel += numMips;
    }

    commandContext.TransitionResource(resource, oldResourceState, true);
}

void D3D12MipGenerator::GenerateMips2D(
    D3D12CommandContext&        commandContext,
    D3D12Texture&               texture,
    ID3D12DescriptorHeap*       mipDescHeap,
    const TextureSubresource&   subresource)
{
    const bool isFormatSRGB = DXTypes::IsDXGIFormatSRGB(texture.GetDXFormat());

    ID3D12GraphicsCommandList* commandList = commandContext.GetCommandList();
    D3D12Resource& resource = texture.GetResource();

    const D3D12_RESOURCE_STATES oldResourceState = resource.currentState;
    commandContext.TransitionResource(resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

    /* Set root signature and descriptor heap */
    commandContext.SetComputeRootSignature(rootSignature2D_.Get());

    ID3D12DescriptorHeap* descHeaps[] = { mipDescHeap };
    commandContext.SetDescriptorHeaps(1, descHeaps);

    D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle = mipDescHeap->GetGPUDescriptorHandleForHeapStart();

    /* Set SRV to read from entire MIP-map chain */
    commandList->SetComputeRootDescriptorTable(1, gpuDescHandle);
    gpuDescHandle.ptr += descHandleSize_;

    D3D12_RESOURCE_DESC resourceDesc = texture.GetNative()->GetDesc();

    const std::uint32_t mipLevelEnd = subresource.baseMipLevel + subresource.numMipLevels - 1;

    for (std::uint32_t mipLevel = subresource.baseMipLevel; mipLevel < mipLevelEnd;)
    {
        /* Determine source and destination extents */
        UINT srcWidth   = static_cast<UINT>(resourceDesc.Width)  >> mipLevel;
        UINT srcHeight  = static_cast<UINT>(resourceDesc.Height) >> mipLevel;

        UINT dstWidth   = std::max(1u, srcWidth  >> 1);
        UINT dstHeight  = std::max(1u, srcHeight >> 1);

        /* Bind pipeline state depending on power-of-two class */
        UINT nonPowerOfTwo = ((srcWidth & 1) | ((srcHeight & 1) << 1));
        if (isFormatSRGB)
            commandContext.SetPipelineState(pipelines2D_[nonPowerOfTwo + 4].Get());
        else
            commandContext.SetPipelineState(pipelines2D_[nonPowerOfTwo].Get());

        /* Determine how many MIP-maps can be downsampled at once; must be in [1, 4] */
        UINT numMips = (mipLevel + 4 >= mipLevelEnd ? mipLevelEnd - mipLevel : 4);

        /* Run compute shader to generate next four MIP-maps */
        commandContext.SetComputeConstant(0, 1.0f / static_cast<float>(dstWidth), 0);
        commandContext.SetComputeConstant(0, 1.0f / static_cast<float>(dstHeight), 1);
        commandContext.SetComputeConstant(0, mipLevel, 2);
        commandContext.SetComputeConstant(0, numMips, 3);
        commandContext.SetComputeConstant(0, subresource.baseArrayLayer, 4);

        commandList->SetComputeRootDescriptorTable(2, gpuDescHandle);
        gpuDescHandle.ptr += descHandleSize_ * numMips;

        commandList->Dispatch(
            DivideRoundUp(dstWidth,  8u),
            DivideRoundUp(dstHeight, 8u),
            subresource.numArrayLayers
        );

        /* Insert UAV barrier and move to next four MIP-maps */
        commandContext.UAVBarrier(texture.GetNative(), true);

        mipLevel += numMips;
    }

    commandContext.TransitionResource(resource, oldResourceState, true);
}

void D3D12MipGenerator::GenerateMips3D(
    D3D12CommandContext&        commandContext,
    D3D12Texture&               texture,
    ID3D12DescriptorHeap*       mipDescHeap,
    const TextureSubresource&   subresource)
{
    const bool isFormatSRGB = DXTypes::IsDXGIFormatSRGB(texture.GetDXFormat());

    ID3D12GraphicsCommandList* commandList = commandContext.GetCommandList();
    D3D12Resource& resource = texture.GetResource();

    const D3D12_RESOURCE_STATES oldResourceState = texture.GetResource().currentState;
    commandContext.TransitionResource(resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

    /* Set root signature and descriptor heap */
    commandContext.SetComputeRootSignature(rootSignature3D_.Get());

    ID3D12DescriptorHeap* descHeaps[] = { mipDescHeap };
    commandContext.SetDescriptorHeaps(1, descHeaps);

    D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle = mipDescHeap->GetGPUDescriptorHandleForHeapStart();

    /* Set SRV to read from entire MIP-map chain */
    commandList->SetComputeRootDescriptorTable(1, gpuDescHandle);
    gpuDescHandle.ptr += descHandleSize_;

    D3D12_RESOURCE_DESC resourceDesc = texture.GetNative()->GetDesc();

    const std::uint32_t mipLevelEnd = subresource.baseMipLevel + subresource.numMipLevels - 1;

    for (std::uint32_t mipLevel = subresource.baseMipLevel; mipLevel < mipLevelEnd;)
    {
        /* Determine source and destination extents */
        UINT srcWidth   = static_cast<UINT>(resourceDesc.Width)            >> mipLevel;
        UINT srcHeight  = static_cast<UINT>(resourceDesc.Height)           >> mipLevel;
        UINT srcDepth   = static_cast<UINT>(resourceDesc.DepthOrArraySize) >> mipLevel;

        UINT dstWidth   = std::max(1u, srcWidth  >> 1);
        UINT dstHeight  = std::max(1u, srcHeight >> 1);
        UINT dstDepth   = std::max(1u, srcDepth  >> 1);

        /* Bind pipeline state depending on power-of-two class */
        UINT nonPowerOfTwo = ((srcWidth & 1) | ((srcHeight & 1) << 1) | ((srcDepth & 1) << 2));
        if (isFormatSRGB)
            commandContext.SetPipelineState(pipelines3D_[nonPowerOfTwo + 8].Get());
        else
            commandContext.SetPipelineState(pipelines3D_[nonPowerOfTwo].Get());

        /* Determine how many MIP-maps can be downsampled at once; must be in [1, 3] */
        UINT numMips = (mipLevel + 3 >= mipLevelEnd ? mipLevelEnd - mipLevel : 3);

        /* Run compute shader to generate next four MIP-maps */
        commandContext.SetComputeConstant(0, 1.0f / static_cast<float>(dstWidth), 0);
        commandContext.SetComputeConstant(0, 1.0f / static_cast<float>(dstHeight), 1);
        commandContext.SetComputeConstant(0, 1.0f / static_cast<float>(dstDepth), 2);
        commandContext.SetComputeConstant(0, mipLevel, 3);
        commandContext.SetComputeConstant(0, numMips, 4);

        commandList->SetComputeRootDescriptorTable(2, gpuDescHandle);
        gpuDescHandle.ptr += descHandleSize_ * numMips;

        commandList->Dispatch(
            DivideRoundUp(dstWidth,  4u),
            DivideRoundUp(dstHeight, 4u),
            DivideRoundUp(dstDepth,  4u)
        );

        /* Insert UAV barrier and move to next four MIP-maps */
        commandContext.UAVBarrier(texture.GetNative(), true);

        mipLevel += numMips;
    }

    commandContext.TransitionResource(resource, oldResourceState, true);
}


} // /namespace LLGL



// ================================================================================
