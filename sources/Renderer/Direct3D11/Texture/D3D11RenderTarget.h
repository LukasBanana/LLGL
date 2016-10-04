/*
 * D3D11RenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D11_RENDER_TARGET_H__
#define __LLGL_D3D11_RENDER_TARGET_H__


#include <LLGL/RenderTarget.h>
#include "../../ComPtr.h"
#include <vector>
#include <functional>
#include <d3d11.h>


namespace LLGL
{


class D3D11Texture;
class D3D11RenderSystem;

class D3D11RenderTarget : public RenderTarget
{

    public:

        D3D11RenderTarget(D3D11RenderSystem& renderSystem, unsigned int multiSamples);

        void AttachDepthBuffer(const Gs::Vector2ui& size) override;
        void AttachStencilBuffer(const Gs::Vector2ui& size) override;
        void AttachDepthStencilBuffer(const Gs::Vector2ui& size) override;

        void AttachTexture(Texture& texture, const RenderTargetAttachmentDescriptor& attachmentDesc) override;

        void DetachAll() override;

        /* ----- Extended Internal Functions ----- */

        inline const std::vector<ID3D11RenderTargetView*>& GetRenderTargetViews() const
        {
            return renderTargetViewsRef_;
        }

        inline ID3D11DepthStencilView* GetDepthStencilView() const
        {
            return depthStencilView_.Get();
        }

    private:

        using AttachTextureCallback = std::function<void(D3D11Texture& textureD3D, D3D11_RENDER_TARGET_VIEW_DESC& desc)>;

        void CreateDepthStencilAndDSV(const Gs::Vector2ui& size, DXGI_FORMAT format);
        void CreateAndAppendRTV(ID3D11Resource* resource, const D3D11_RENDER_TARGET_VIEW_DESC& desc);

        bool HasMultiSampling() const;

        D3D11RenderSystem&                          renderSystem_;

        std::vector<ComPtr<ID3D11RenderTargetView>> renderTargetViews_;
        std::vector<ID3D11RenderTargetView*>        renderTargetViewsRef_;

        ComPtr<ID3D11Texture2D>                     depthStencil_;
        ComPtr<ID3D11DepthStencilView>              depthStencilView_;

        UINT                                        multiSamples_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
