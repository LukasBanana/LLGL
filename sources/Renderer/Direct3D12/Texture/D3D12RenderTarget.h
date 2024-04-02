/*
 * D3D12RenderTarget.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_RENDER_TARGET_H
#define LLGL_D3D12_RENDER_TARGET_H


#include <LLGL/RenderTarget.h>
#include <LLGL/Container/SmallVector.h>
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

        #include <LLGL/Backend/RenderTarget.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        D3D12RenderTarget(D3D12Device& device, const RenderTargetDescriptor& desc);

        void TransitionToOutputMerger(D3D12CommandContext& commandContext);
        void ResolveSubresources(D3D12CommandContext& commandContext);

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForRTV() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForDSV() const;

        // Returns true if this render-target has multi-sampled color attachments.
        inline bool HasMultiSampling() const
        {
            return (sampleDesc_.Count > 1);
        }

    private:

        using ColorFormatVector = SmallVector<DXGI_FORMAT, LLGL_MAX_NUM_COLOR_ATTACHMENTS>;

        UINT GatherAttachmentFormats(D3D12Device& device, const RenderTargetDescriptor& desc, ColorFormatVector& outColorFormats);

        void CreateDescriptorHeaps(ID3D12Device* device, const UINT numColorTargets);

        void CreateAttachments(
            ID3D12Device*                   device,
            const RenderTargetDescriptor&   desc,
            const ColorFormatVector&        colorFormats
        );

        void CreateColorAttachment(
            ID3D12Device*                   device,
            const AttachmentDescriptor&     colorAttachment,
            const AttachmentDescriptor&     resolveAttachment,
            DXGI_FORMAT                     format,
            D3D12_CPU_DESCRIPTOR_HANDLE     cpuDescHandle
        );

        void CreateDepthStencilAttachment(
            ID3D12Device*                   device,
            const AttachmentDescriptor&     depthStenciAttachment,
            D3D12_CPU_DESCRIPTOR_HANDLE     cpuDescHandle,
            D3D12_DSV_FLAGS                 dsvFlags                = D3D12_DSV_FLAG_NONE
        );

        D3D12Resource* CreateInternalTexture(
            ID3D12Device*               device,
            DXGI_FORMAT                 format,
            D3D12_RESOURCE_STATES       initialState,
            D3D12_RESOURCE_FLAGS        flags,
            const D3D12_CLEAR_VALUE*    clearValue = nullptr
        );

        void CreateRenderTargetView(
            ID3D12Device*               device,
            D3D12Resource&              resource,
            DXGI_FORMAT                 format,
            const TextureType           type,
            UINT                        mipLevel,
            UINT                        arrayLayer,
            D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle
        );

        void CreateDepthStencilView(
            ID3D12Device*       device,
            D3D12Resource&      resource,
            DXGI_FORMAT         format,
            const TextureType   type,
            UINT                mipLevel,
            UINT                arrayLayer,
            D3D12_DSV_FLAGS     dsvFlags    = D3D12_DSV_FLAG_NONE
        );

        void CreateResolveTarget(
            const AttachmentDescriptor& resolveAttachment,
            DXGI_FORMAT                 format,
            D3D12Resource*              multiSampledSrcTexture
        );

    private:

        // Members for multi-sampled render-targets
        struct ResolveTarget
        {
            D3D12Resource*  resolveDstTexture;
            UINT            resolveDstSubresource;
            D3D12Resource*  multiSampledSrcTexture;
            DXGI_FORMAT     format;
        };

    private:

        Extent2D                        resolution_;
        DXGI_SAMPLE_DESC                sampleDesc_         = { 1, 0 };

        // Objects:
        ComPtr<ID3D12DescriptorHeap>    rtvDescHeap_;
        ComPtr<ID3D12DescriptorHeap>    dsvDescHeap_;
        DXGI_FORMAT                     depthStencilFormat_ = DXGI_FORMAT_UNKNOWN;
        D3D12RenderPass                 defaultRenderPass_;

        // Containers and references:
        std::vector<D3D12Resource>      internalTextures_;
        std::vector<D3D12Resource*>     colorBuffers_;
        std::vector<ResolveTarget>      resolveTargets_;
        D3D12Resource*                  depthStencil_       = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
