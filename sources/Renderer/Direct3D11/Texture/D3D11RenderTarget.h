/*
 * D3D11RenderTarget.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_RENDER_TARGET_H
#define LLGL_D3D11_RENDER_TARGET_H


#include <LLGL/RenderTarget.h>
#include "D3D11RenderTargetHandles.h"
#include "../../DXCommon/ComPtr.h"
#include "../../../Core/CompilerExtensions.h"
#include <vector>
#include <functional>
#include <d3d11.h>


namespace LLGL
{


class D3D11Texture;
class D3D11RenderPass;
class D3D11RenderSystem;

class D3D11RenderTarget final : public RenderTarget
{

    public:

        #include <LLGL/Backend/RenderTarget.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        // Constructs the D3D11 render target with all attachments.
        D3D11RenderTarget(ID3D11Device* device, const RenderTargetDescriptor& desc);

        // Resolves all multi-sampled subresources.
        void ResolveSubresources(ID3D11DeviceContext* context);

        // Returns the render-target handles container.
        inline const D3D11RenderTargetHandles& GetRenderTargetHandles() const
        {
            return renderTargetHandles_;
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
            UINT                        numArrayLayers  = 1,
            UINT                        dsvFlags        = 0
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

        LLGL_NODISCARD
        ComPtr<ID3D11RenderTargetView> CreateRenderTargetView(
            ID3D11Device*               device,
            const AttachmentDescriptor& colorAttachment,
            const AttachmentDescriptor& resolveAttachment,
            D3D11BindingLocator*&       outBindingLocator,
            D3D11SubresourceRange&      outSubresourceRange
        );

        LLGL_NODISCARD
        ComPtr<ID3D11DepthStencilView> CreateDepthStencilView(
            ID3D11Device*               device,
            const AttachmentDescriptor& depthStencilAttachment,
            UINT                        dsvFlags,
            D3D11BindingLocator*&       outBindingLocator
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

        std::vector<ComPtr<ID3D11Texture2D>>    internalTextures_;  // Depth-stencil texture or multi-sampled color targets
        DXGI_FORMAT                             depthStencilFormat_     = DXGI_FORMAT_UNKNOWN;
        D3D11RenderTargetHandles                renderTargetHandles_;

        DXGI_SAMPLE_DESC                        sampleDesc_             = { 1u, 0u };
        std::vector<ResolveTarget>              resolveTargets_;

        const D3D11RenderPass*                  renderPass_             = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
