/*
 * D3D12RenderPass.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_RENDER_PASS_H
#define LLGL_D3D12_RENDER_PASS_H


#include <LLGL/RenderPass.h>
#include <LLGL/RenderPassFlags.h>
#include <LLGL/ForwardDecls.h>
#include <LLGL/Constants.h>
#include <LLGL/Container/ArrayView.h>
#include <cstdint>
#include <cstddef>
#include <d3d12.h>
#include <algorithm>


namespace LLGL
{


class D3D12Device;

class D3D12RenderPass final : public RenderPass
{

    public:

        D3D12RenderPass();

        // Constructs the render pass with the specified descriptor and uses the device to find a suitable sample descriptor (i.e DXGI_SAMPLE_DESC).
        D3D12RenderPass(
            const D3D12Device&          device,
            const RenderPassDescriptor& desc
        );

        // Builds the color and depth-stencil attachments index and format buffers with the specified render pass descriptor.
        void BuildAttachments(
            const D3D12Device&          device,
            const RenderPassDescriptor& desc
        );

        // Builds the attachments with the explicit DXGI_FORMAT entries for color and depth-stencil.
        void BuildAttachments(
            UINT                    numColorFormats,
            const DXGI_FORMAT*      colorFormats,
            const DXGI_FORMAT       depthStencilFormat,
            const DXGI_SAMPLE_DESC& sampleDesc,
            UINT                    numViews            = 1
        );

        // Returns the number of color attachments used for this render pass.
        inline UINT GetNumColorAttachments() const
        {
            return numColorAttachments_;
        }

        // Specifies the clear flags for the depth-stencil view (DSV).
        inline D3D12_CLEAR_FLAGS GetClearFlagsDSV() const
        {
            return static_cast<D3D12_CLEAR_FLAGS>(clearFlagsDSV_);
        }

        // Returns the array of color attachment indices that are meant to be cleared when a render pass begins (value of 0xFF ends the list).
        inline const std::uint8_t* GetClearColorAttachments() const
        {
            return clearColorAttachments_;
        }

        // Returns a bitwise OR combination of D3D12_DSV_FLAGS entries.
        inline D3D12_DSV_FLAGS GetAttachmentFlagsDSV() const
        {
            return static_cast<D3D12_DSV_FLAGS>(attachmentFlagsDSV_);
        }

        // Returns the array of native color formats.
        inline const DXGI_FORMAT* GetRTVFormats() const
        {
            return rtvFormats_;
        }

        // Returns the native depth-stencil format.
        inline DXGI_FORMAT GetDSVFormat() const
        {
            return dsvFormat_;
        }

        // Returns the native sample descriptor.
        inline const DXGI_SAMPLE_DESC& GetSampleDesc() const
        {
            return sampleDesc_;
        }

        // Returns the number of views for multiview (view-instanced) rendering, or 1 if multiview is disabled.
        inline UINT GetNumViews() const
        {
            return numViews_;
        }

        // Returns the resolve modes for all color attachments.
        inline ArrayView<ResolveMode> GetColorResolveModes() const
        {
            return ArrayView<ResolveMode>{ resolveModes_, numColorAttachments_ };
        }

        // Returns the resolve mode for the depth attachment.
        inline ResolveMode GetDepthResolveMode() const
        {
            return resolveModes_[LLGL_MAX_NUM_COLOR_ATTACHMENTS];
        }

        // Returns the resolve mode for the stencil attachment.
        inline ResolveMode GetStencilResolveMode() const
        {
            return resolveModes_[LLGL_MAX_NUM_COLOR_ATTACHMENTS + 1];
        }

    private:

        void SetDSVFormat(DXGI_FORMAT format);
        void SetRTVFormat(DXGI_FORMAT format, UINT colorAttachment);

    private:

        UINT                numColorAttachments_    : 20;
        UINT                numViews_               :  8;
        UINT                clearFlagsDSV_          :  2;
        UINT                attachmentFlagsDSV_     :  2;

        std::uint8_t        clearColorAttachments_[LLGL_MAX_NUM_COLOR_ATTACHMENTS]  = {};

        DXGI_FORMAT         rtvFormats_[LLGL_MAX_NUM_COLOR_ATTACHMENTS]             = {};
        DXGI_FORMAT         dsvFormat_                                              = DXGI_FORMAT_UNKNOWN;

        ResolveMode         resolveModes_[LLGL_MAX_NUM_COLOR_ATTACHMENTS + 2]       = {};

        DXGI_SAMPLE_DESC    sampleDesc_                                             = { 1, 0 };

};

inline D3D12_DSV_FLAGS GetD3D12RenderPassAttachmentFlagsDSV(const D3D12RenderPass* renderPassD3D)
{
    return (renderPassD3D != nullptr ? renderPassD3D->GetAttachmentFlagsDSV() : D3D12_DSV_FLAG_NONE);
}

inline ResolveMode GetD3D12RenderPassColorResolveMode(const D3D12RenderPass* renderPassD3D, std::size_t colorAttachmentIndex)
{
    return (renderPassD3D != nullptr ? renderPassD3D->GetColorResolveModes()[colorAttachmentIndex] : ResolveMode::Average);
}

inline ResolveMode GetD3D12RenderPassDepthResolveMode(const D3D12RenderPass* renderPassD3D)
{
    return (renderPassD3D != nullptr ? renderPassD3D->GetDepthResolveMode() : ResolveMode::Min);
}

inline ResolveMode GetD3D12RenderPassStencilResolveMode(const D3D12RenderPass* renderPassD3D)
{
    return (renderPassD3D != nullptr ? renderPassD3D->GetStencilResolveMode() : ResolveMode::Min);
}

inline UINT GetD3D12RenderPassNumViewsOrDefault(const D3D12RenderPass* renderPassD3D, UINT numViewsByDefault)
{
    return (renderPassD3D != nullptr ? renderPassD3D->GetNumViews() : std::max<UINT>(1u, numViewsByDefault));
}


} // /namespace LLGL


#endif



// ================================================================================
