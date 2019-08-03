/*
 * D3D12RenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_RENDER_TARGET_H
#define LLGL_D3D12_RENDER_TARGET_H


#include <LLGL/RenderTarget.h>
#include "../D3D12Resource.h"
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

        void CreateDepthStencil(ID3D12Device* device, DXGI_FORMAT format);

    private:

        Extent2D                        resolution_;

        // Objects:
        ComPtr<ID3D12DescriptorHeap>    rtvDescHeap_;
        UINT                            rtvDescSize_        = 0;
        std::vector<DXGI_FORMAT>        colorFormats_;

        ComPtr<ID3D12DescriptorHeap>    dsvDescHeap_;
        D3D12Resource                   depthStencilBuffer_;
        DXGI_FORMAT                     depthStencilFormat_ = DXGI_FORMAT_UNKNOWN;

        // References:
        std::vector<D3D12Resource*>     colorBuffers_;
        D3D12Resource*                  depthStencil_       = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
