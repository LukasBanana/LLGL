/*
 * D3D11RenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

        D3D11RenderTarget(ID3D11Device* device, const RenderTargetDescriptor& desc);

        Extent2D GetResolution() const override;
        std::uint32_t GetNumColorAttachments() const override;
        bool HasDepthAttachment() const override;
        bool HasStencilAttachment() const override;

        /* ----- Extended Internal Functions ----- */

        // Resolves all multi-sampled subresources.
        void ResolveSubresources(ID3D11DeviceContext* context);

        // Returns the list of native render target views (RTV).
        inline const std::vector<ID3D11RenderTargetView*>& GetRenderTargetViews() const
        {
            return renderTargetViewsRef_;
        }

        // Returns the native depth stencil view (DSV).
        inline ID3D11DepthStencilView* GetDepthStencilView() const
        {
            return depthStencilView_.Get();
        }

    private:

        void Attach(const AttachmentDescriptor& attachmentDesc);
        void AttachDepthBuffer();
        void AttachStencilBuffer();
        void AttachDepthStencilBuffer();
        void AttachTexture(Texture& texture, const AttachmentDescriptor& attachmentDesc);

        void CreateDepthStencilAndDSV(DXGI_FORMAT format);
        void CreateAndAppendRTV(ID3D11Resource* resource, const D3D11_RENDER_TARGET_VIEW_DESC& rtvDesc);

        bool HasMultiSampling() const;

        ID3D11Device*                               device_                     = nullptr;

        Extent2D                                    resolution_;

        std::vector<ComPtr<ID3D11RenderTargetView>> renderTargetViews_;
        std::vector<ID3D11RenderTargetView*>        renderTargetViewsRef_;

        ComPtr<ID3D11Texture2D>                     depthStencil_;
        ComPtr<ID3D11DepthStencilView>              depthStencilView_;
        DXGI_FORMAT                                 depthStencilFormat_         = DXGI_FORMAT_UNKNOWN;

        // Members for multi-sampled render-targets
        struct MultiSampledAttachment
        {
            ComPtr<ID3D11Texture2D> texture2DMS;
            ID3D11Texture2D*        targetTexture;
            UINT                    targetSubresourceIndex;
            DXGI_FORMAT             format;
        };

        UINT                                        multiSamples_               = 0;
        std::vector<MultiSampledAttachment>         multiSampledAttachments_;

};


} // /namespace LLGL


#endif



// ================================================================================
