/*
 * D3D12RenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_RENDER_TARGET_H
#define LLGL_D3D12_RENDER_TARGET_H


#include <LLGL/RenderTarget.h>
#include "../D3D12Resource.h"
#include "../RenderState/D3D12RenderPass.h"
#include <d3d12.h>
#include "../../DXCommon/ComPtr.h"


namespace LLGL
{


class D3D12Device;
class D3D12Texture;
class D3D12CommandContext;

class D3D12RenderTarget final : public RenderTarget
{

    public:

        void SetName(const char* name) override;

        Extent2D GetResolution() const override;
        std::uint32_t GetSamples() const override;
        std::uint32_t GetNumColorAttachments() const override;

        bool HasDepthAttachment() const override;
        bool HasStencilAttachment() const override;

        const RenderPass* GetRenderPass() const override;

    public:

        D3D12RenderTarget(D3D12Device& device, const RenderTargetDescriptor& desc);

        void TransitionToOutputMerger(D3D12CommandContext& commandContext);
        void ResolveRenderTarget(D3D12CommandContext& commandContext);

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForRTV() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForDSV() const;

        bool HasMultiSampling() const;

    private:

        void CreateDescriptorHeaps(D3D12Device& device, const RenderTargetDescriptor& desc);
        void CreateAttachments(ID3D12Device* device, const RenderTargetDescriptor& desc);
        void CreateColorBuffersMS(ID3D12Device* device, const RenderTargetDescriptor& desc, D3D12_CPU_DESCRIPTOR_HANDLE& cpuDescHandle);
        void CreateDepthStencil(ID3D12Device* device, DXGI_FORMAT format);

        void CreateSubresource(
            ID3D12Device*                   device,
            const AttachmentType            attachmentType,
            D3D12Resource&                  resource,
            DXGI_FORMAT                     format,
            const TextureType               textureType,
            UINT                            mipLevel,
            UINT                            arrayLayer,
            D3D12_CPU_DESCRIPTOR_HANDLE&    cpuDescHandle
        );

        void CreateSubresourceRTV(
            ID3D12Device*                       device,
            D3D12Resource&                      resource,
            DXGI_FORMAT                         format,
            const TextureType                   type,
            UINT                                mipLevel,
            UINT                                arrayLayer,
            const D3D12_CPU_DESCRIPTOR_HANDLE&  cpuDescHandle
        );

        void CreateSubresourceDSV(
            ID3D12Device*       device,
            D3D12Resource&      resource,
            DXGI_FORMAT         format,
            const TextureType   type,
            UINT                mipLevel,
            UINT                arrayLayer
        );

    private:

        struct ColorBufferMS
        {
            D3D12Resource   resource;
            UINT            dstSubresource;
        };

    private:

        Extent2D                        resolution_;
        DXGI_SAMPLE_DESC                sampleDesc_         = { 1, 0 };

        // Objects:
        ComPtr<ID3D12DescriptorHeap>    rtvDescHeap_;
        UINT                            rtvDescSize_        = 0;

        ComPtr<ID3D12DescriptorHeap>    dsvDescHeap_;
        D3D12Resource                   depthStencilBuffer_;
        DXGI_FORMAT                     depthStencilFormat_ = DXGI_FORMAT_UNKNOWN;
        D3D12RenderPass                 defaultRenderPass_;

        // Containers and references:
        std::vector<DXGI_FORMAT>        colorFormats_;
        std::vector<D3D12Resource*>     colorBuffers_;
        std::vector<ColorBufferMS>      colorBuffersMS_;
        D3D12Resource*                  depthStencil_       = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
