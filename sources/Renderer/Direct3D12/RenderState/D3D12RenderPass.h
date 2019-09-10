/*
 * D3D12RenderPass.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_RENDER_PASS_H
#define LLGL_D3D12_RENDER_PASS_H


#include <LLGL/RenderPass.h>
#include <LLGL/ForwardDecls.h>
#include "../../StaticLimits.h"
#include <cstdint>
#include <cstddef>
#include <d3d12.h>


namespace LLGL
{


class D3D12RenderPass final : public RenderPass
{

    public:

        D3D12RenderPass() = default;
        D3D12RenderPass(const RenderPassDescriptor& desc);

        // Builds the color and depth-stencil attachments index and format buffers with the specified render pass descriptor.
        void BuildAttachments(const RenderPassDescriptor& desc);

        // Builds the color and depth-stencil attachments index and format buffers with the specified render target attachment descriptor.
        void BuildAttachments(
            UINT                        numAttachmentDescs,
            const AttachmentDescriptor* attachmentDescs,
            const DXGI_FORMAT           defaultDepthStencilFormat
        );

        // Builds the attachments with the explicit DXGI_FORMAT entries for color and depth-stencil.
        void BuildAttachments(
            UINT                numColorFormats,
            const DXGI_FORMAT*  colorFormats,
            const DXGI_FORMAT   depthStencilFormat
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

    private:

        void SetDSVFormat(DXGI_FORMAT format);
        void SetRTVFormat(UINT colorAttachment, DXGI_FORMAT format);

    private:

        UINT            numColorAttachments_                                    = 0;

        UINT            clearFlagsDSV_                                          = 0;
        std::uint8_t    clearColorAttachments_[LLGL_MAX_NUM_COLOR_ATTACHMENTS]  = {};

        DXGI_FORMAT     rtvFormats_[LLGL_MAX_NUM_COLOR_ATTACHMENTS]             = {};
        DXGI_FORMAT     dsvFormat_                                              = DXGI_FORMAT_UNKNOWN;

};


} // /namespace LLGL


#endif



// ================================================================================
