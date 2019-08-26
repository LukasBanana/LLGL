/*
 * D3D12MipGenerator.cpp
 \*

 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12MipGenerator.h"
#include "D3D12Texture.h"
#include "../Shader/Builtin/D3D12Builtin.h"
#include "../Command/D3D12CommandContext.h"
#include "../D3DX12/d3dx12.h"
#include "../../DXCommon/DXCore.h"
#include "../../DXCommon/DXTypes.h"


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

    /* Initialize linear sampler */
    linearSamplerDesc_.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    linearSamplerDesc_.SetTextureAddressModes(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

    /* Initialize root signature */
    D3D12RootSignature rootSignature;
    {
        rootSignature.ResetAndAlloc(3, 1);
        rootSignature[0].InitAsConstants(0, 4);
        rootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
        rootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 4);
        rootSignature.AppendStaticSampler();
    }
    rootSignature_ = rootSignature.Finalize(device);

    /* Create compute PSOs */
    CreateComputePSO(device, 0, LLGL_IDR_GENERATEMIPS_CS           );
    CreateComputePSO(device, 1, LLGL_IDR_GENERATEMIPS_CS_ODDX      );
    CreateComputePSO(device, 2, LLGL_IDR_GENERATEMIPS_CS_ODDY      );
    CreateComputePSO(device, 3, LLGL_IDR_GENERATEMIPS_CS_ODDXY     );
    CreateComputePSO(device, 4, LLGL_IDR_GENERATEMIPS_CS_SRGB      );
    CreateComputePSO(device, 5, LLGL_IDR_GENERATEMIPS_CS_SRGB_ODDX );
    CreateComputePSO(device, 6, LLGL_IDR_GENERATEMIPS_CS_SRGB_ODDY );
    CreateComputePSO(device, 7, LLGL_IDR_GENERATEMIPS_CS_SRGB_ODDXY);
}

void D3D12MipGenerator::Clear()
{
    for (auto& pso : pipelines_)
        pso.Reset();
    rootSignature_.Reset();
}

void D3D12MipGenerator::GenerateMips(
    D3D12CommandContext&        commandContext,
    D3D12Texture&               texture,
    const TextureSubresource&   subresource)
{
    if (texture.SupportsGenerateMips())
    {
        switch (texture.GetType())
        {
            case TextureType::Texture1D:
            case TextureType::Texture1DArray:
                //TODO
                break;
            case TextureType::Texture2D:
            case TextureType::TextureCube:
            case TextureType::Texture2DArray:
            case TextureType::TextureCubeArray:
                GenerateMips2D(commandContext, texture, subresource);
                break;
            case TextureType::Texture3D:
                //TODO
                break;
            case TextureType::Texture2DMS:
            case TextureType::Texture2DMSArray:
                // no MIP-maps for multi-sampled textures
                break;
        }
    }
}


/*
 * ======= Private: =======
 */

void D3D12MipGenerator::CreateComputePSO(ID3D12Device* device, std::size_t index, int resourceID)
{
    if (auto blob = DXCreateBlobFromResource(resourceID))
    {
        /* Create graphics pipeline state and graphics command list */
        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
        {
            psoDesc.pRootSignature      = rootSignature_.Get();
            psoDesc.CS.pShaderBytecode  = blob->GetBufferPointer();
            psoDesc.CS.BytecodeLength   = blob->GetBufferSize();
        }
        auto hr = device_->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(pipelines_[index].ReleaseAndGetAddressOf()));
        DXThrowIfCreateFailed(hr, "ID3D12PipelineState");
    }
}

void D3D12MipGenerator::GenerateMips2D(
    D3D12CommandContext&        commandContext,
    D3D12Texture&               texture,
    const TextureSubresource&   subresource)
{
    if (subresource.numMipLevels == 0 || subresource.numArrayLayers == 0)
        return;

    auto mipDescHeap = texture.GetMipDescHeap();
    if (mipDescHeap == nullptr)
        return;

    const auto format       = texture.GetFormat();
    const bool isFormatSRGB = DXTypes::IsDXGIFormatSRGB(format);

    auto& resource = texture.GetResource();
    ID3D12GraphicsCommandList* commandList = commandContext.GetCommandList();

    commandContext.TransitionResource(resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

    /* Set root signature and descriptor heap */
    commandList->SetComputeRootSignature(rootSignature_.Get());

    ID3D12DescriptorHeap* descHeaps[] = { mipDescHeap };
    commandList->SetDescriptorHeaps(1, descHeaps);

    auto gpuDescHandle = mipDescHeap->GetGPUDescriptorHandleForHeapStart();

    /* Set SRV to read from entire MIP-map chain */
    commandList->SetComputeRootDescriptorTable(1, gpuDescHandle);
    gpuDescHandle.ptr += descHandleSize_;

    D3D12_RESOURCE_DESC resourceDesc = resource.native->GetDesc();

    const auto mipLevelEnd = subresource.baseMipLevel + subresource.numMipLevels;

    for (std::uint32_t mipLevel = subresource.baseMipLevel; mipLevel < mipLevelEnd;)
    {
        /* Determine source and destination extents */
        UINT srcWidth   = static_cast<UINT>(resourceDesc.Width) >> mipLevel;
        UINT srcHeight  = resourceDesc.Height >> mipLevel;
        UINT dstWidth   = std::max(1u, srcWidth >> 1);
        UINT dstHeight  = std::max(1u, srcHeight >> 1);

        /* Bind pipeline state depending on power-of-two class */
        UINT nonPowerOfTwo = ((srcWidth & 1) | ((srcHeight & 1) << 1));
        if (isFormatSRGB)
            commandList->SetPipelineState(pipelines_[nonPowerOfTwo + 4].Get());
        else
            commandList->SetPipelineState(pipelines_[nonPowerOfTwo].Get());

        /* Determine how many MIP-maps can be downsampled at once; must be in [1, 4] */
        UINT numMips = (mipLevel + 4 >= mipLevelEnd ? mipLevelEnd - mipLevel : 4);

        /* Run compute shader to generate next four MIP-maps */
        commandContext.SetComputeConstant(0, mipLevel, 0);
        commandContext.SetComputeConstant(0, numMips, 1);
        commandContext.SetComputeConstant(0, 1.0f / static_cast<float>(dstWidth), 2);
        commandContext.SetComputeConstant(0, 1.0f / static_cast<float>(dstHeight), 3);

        commandList->SetComputeRootDescriptorTable(2, gpuDescHandle);
        gpuDescHandle.ptr += descHandleSize_ * numMips;

        commandList->Dispatch(
            std::max(1u, dstWidth  / 8u),
            std::max(1u, dstHeight / 8u),
            1
        );

        /* Insert UAV barrier and move to next four MIP-maps */
        commandContext.InsertUAVBarrier(resource, true);

        mipLevel += numMips;
    }

    commandContext.TransitionResource(resource, resource.usageState, true);
}


} // /namespace LLGL



// ================================================================================
