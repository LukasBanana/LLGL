/*
 * D3D11RenderTarget.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_RENDER_TARGET_H
#define LLGL_D3D11_RENDER_TARGET_H


#include <LLGL/RenderTarget.h>
#include "../../DXCommon/ComPtr.h"
#include <vector>
#include <functional>
#include <d3d11.h>


namespace LLGL
{


class D3D11Texture;
class D3D11RenderSystem;

class D3D11RenderTarget final : public RenderTarget
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

        // Constructs the D3D11 render target with all attachments.
        D3D11RenderTarget(ID3D11Device* device, const RenderTargetDescriptor& desc);

        // Releases all render target views manually to avoid having a separate container with ComPtr.
        ~D3D11RenderTarget();

        // Resolves all multi-sampled subresources.
        void ResolveSubresources(ID3D11DeviceContext* context);

        // Returns the list of native render target views (RTV).
        inline const std::vector<ID3D11RenderTargetView*>& GetRenderTargetViews() const
        {
            return renderTargetViews_;
        }

        // Returns the native depth stencil view (DSV).
        inline ID3D11DepthStencilView* GetDepthStencilView() const
        {
            return depthStencilView_.Get();
        }

        // Returns true if this render-target has multi-sampled color attachments.
        inline bool HasMultiSampling() const
        {
            return (sampleDesc_.Count > 1);
        }

    public:

        // Creates a depth-stencil-view (DSV) of the specified subresource.
        static void CreateSubresourceDSV(
            ID3D11Device*               device,
            ID3D11Resource*             resource,
            ID3D11DepthStencilView**    dsvOutput,
            const TextureType           type,
            const DXGI_FORMAT           format,
            UINT                        baseMipLevel,
            UINT                        baseArrayLayer,
            UINT                        numArrayLayers = 1
        );

        // Creates a render-target-view (RTV) of the specified subresource.
        static void CreateSubresourceRTV(
            ID3D11Device*               device,
            ID3D11Resource*             resource,
            ID3D11RenderTargetView**    rtvOutput,
            const TextureType           type,
            const DXGI_FORMAT           format,
            UINT                        baseMipLevel,
            UINT                        baseArrayLayer,
            UINT                        numArrayLayers = 1
        );

    private:

        void FindSuitableSampleDesc(ID3D11Device* device, const RenderTargetDescriptor& desc);

        ID3D11Texture2D* CreateInternalTexture(ID3D11Device* device, DXGI_FORMAT format, UINT bindFlags);

        void CreateRenderTargetView(
            ID3D11Device*               device,
            const AttachmentDescriptor& colorAttachment,
            const AttachmentDescriptor& resolveAttachment
        );

        void CreateDepthStencilView(
            ID3D11Device*               device,
            const AttachmentDescriptor& depthStencilAttachment
        );

        void CreateResolveTarget(
            ID3D11Device*               device,
            const AttachmentDescriptor& resolveAttachment,
            DXGI_FORMAT                 format,
            ID3D11Resource*             multiSampledSrcTexture
        );

    private:

        // Members for multi-sampled render-targets
        struct ResolveTarget
        {
            ID3D11Resource* resolveDstTexture;
            UINT            resolveDstSubresource;
            ID3D11Resource* multiSampledSrcTexture;
            DXGI_FORMAT     format;
        };

    private:

        Extent2D                                resolution_;

        std::vector<ID3D11RenderTargetView*>    renderTargetViews_; // Manual calls to IUnknown::Release to avoid having seprate container with ComPtr.
        std::vector<ComPtr<ID3D11Texture2D>>    internalTextures_;  // Depth-stencil texture or multi-sampled color targets

        ComPtr<ID3D11DepthStencilView>          depthStencilView_;
        DXGI_FORMAT                             depthStencilFormat_ = DXGI_FORMAT_UNKNOWN;

        DXGI_SAMPLE_DESC                        sampleDesc_         = { 1u, 0u };
        std::vector<ResolveTarget>              resolveTargets_;

        const RenderPass*                       renderPass_         = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
