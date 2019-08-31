/*
 * D3D12MipGenerator.h
 \*

 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_MIP_GENERATOR_H
#define LLGL_D3D12_MIP_GENERATOR_H


#include <LLGL/Texture.h>
#include <d3d12.h>
#include "../Shader/D3D12RootSignature.h"
#include "D3D12SamplerDesc.h"
#include "../Shader/D3D12Shader.h"
#include "../../DXCommon/ComPtr.h"


namespace LLGL
{


class D3D12Texture;
class D3D12CommandContext;
struct D3D12Resource;
struct TextureSubresource;

// Direct3D 12 MIP-map generator singleton.
class D3D12MipGenerator
{

    public:

        // Returns the singleton instance.
        static D3D12MipGenerator& Get();

    public:

        D3D12MipGenerator(const D3D12MipGenerator&) = delete;
        D3D12MipGenerator& operator = (const D3D12MipGenerator&) = delete;

        D3D12MipGenerator(D3D12MipGenerator&&) = delete;
        D3D12MipGenerator& operator = (D3D12MipGenerator&&) = delete;

        void InitializeDevice(ID3D12Device* device);
        void Clear();

        HRESULT GenerateMips(
            D3D12CommandContext&        commandContext,
            D3D12Texture&               texture,
            const TextureSubresource&   subresource
        );

    private:

        D3D12MipGenerator() = default;

        ComPtr<ID3D12PipelineState> CreateComputePSO(
            ID3D12Device*           device,
            ID3D12RootSignature*    rootSignature,
            int                     resourceID
        );

        void CreateResourcesFor1DMips(ID3D12Device* device);
        void CreateResourcesFor2DMips(ID3D12Device* device);
        void CreateResourcesFor3DMips(ID3D12Device* device);

        void GenerateMips1D(
            D3D12CommandContext&        commandContext,
            D3D12Resource&              resource,
            ID3D12DescriptorHeap*       mipDescHeap,
            DXGI_FORMAT                 format,
            const TextureSubresource&   subresource
        );

        void GenerateMips2D(
            D3D12CommandContext&        commandContext,
            D3D12Resource&              resource,
            ID3D12DescriptorHeap*       mipDescHeap,
            DXGI_FORMAT                 format,
            const TextureSubresource&   subresource
        );

        void GenerateMips3D(
            D3D12CommandContext&        commandContext,
            D3D12Resource&              resource,
            ID3D12DescriptorHeap*       mipDescHeap,
            DXGI_FORMAT                 format,
            const TextureSubresource&   subresource
        );

    private:

        ID3D12Device*               device_             = nullptr;

        ComPtr<ID3D12RootSignature> rootSignature1D_;
        ComPtr<ID3D12PipelineState> pipelines1D_[4];

        ComPtr<ID3D12RootSignature> rootSignature2D_;
        ComPtr<ID3D12PipelineState> pipelines2D_[8];

        ComPtr<ID3D12RootSignature> rootSignature3D_;
        ComPtr<ID3D12PipelineState> pipelines3D_[16];

        UINT                        descHandleSize_     = 0;


};


} // /namespace LLGL


#endif



// ================================================================================
