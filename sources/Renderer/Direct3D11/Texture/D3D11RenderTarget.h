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
#include <d3d11.h>


namespace LLGL
{


class D3D11RenderTarget : public RenderTarget
{

    public:

        D3D11RenderTarget(ID3D11Device* device, unsigned int multiSamples);

        void AttachDepthBuffer(const Gs::Vector2i& size) override;
        void AttachStencilBuffer(const Gs::Vector2i& size) override;
        void AttachDepthStencilBuffer(const Gs::Vector2i& size) override;

        void AttachTexture1D(Texture& texture, int mipLevel = 0) override;
        void AttachTexture2D(Texture& texture, int mipLevel = 0) override;
        void AttachTexture3D(Texture& texture, int layer, int mipLevel = 0) override;
        void AttachTextureCube(Texture& texture, const AxisDirection cubeFace, int mipLevel = 0) override;
        void AttachTexture1DArray(Texture& texture, int layer, int mipLevel = 0) override;
        void AttachTexture2DArray(Texture& texture, int layer, int mipLevel = 0) override;
        void AttachTextureCubeArray(Texture& texture, int layer, const AxisDirection cubeFace, int mipLevel = 0) override;

        void DetachTextures() override;

        /* ----- Extended Internal Functions ----- */

        inline const std::vector<ID3D11RenderTargetView*>& GetRTVList() const
        {
            return rtvRefList_;
        }

        inline ID3D11DepthStencilView* GetDSV() const
        {
            return dsv_.Get();
        }

    private:

        std::vector<ComPtr<ID3D11RenderTargetView>> rtvList_;
        std::vector<ID3D11RenderTargetView*>        rtvRefList_;
        ComPtr<ID3D11DepthStencilView>              dsv_;

};


} // /namespace LLGL


#endif



// ================================================================================
