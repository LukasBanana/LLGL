/*
 * D3D12RenderPass.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_RENDER_PASS_H
#define LLGL_D3D12_RENDER_PASS_H


#include <LLGL/RenderPass.h>
#include <LLGL/ForwardDecls.h>
#include <LLGL/Constants.h>
#include <cstdint>
#include <cstddef>
#include <d3d12.h>


namespace LLGL
{


class D3D12Device;

class D3D12RenderPass final : public RenderPass
{

    public:

        D3D12RenderPass() = default;

        // Constructs the render pass with the specified descriptor and uses the device to find a suitable sample descriptor (i.e DXGI_SAMPLE_DESC).
        D3D12RenderPass(
            const D3D12Device& device,
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
            const DXGI_SAMPLE_DESC& sampleDesc
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
            return attachmentFlagsDSV_;
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

    private:

        void SetDSVFormat(DXGI_FORMAT format);
        void SetRTVFormat(DXGI_FORMAT format, UINT colorAttachment);

    private:

        UINT                numColorAttachments_                                    = 0;

        UINT                clearFlagsDSV_                                          = 0;
        std::uint8_t        clearColorAttachments_[LLGL_MAX_NUM_COLOR_ATTACHMENTS]  = {};
        D3D12_DSV_FLAGS     attachmentFlagsDSV_                                     = D3D12_DSV_FLAG_NONE;

        DXGI_FORMAT         rtvFormats_[LLGL_MAX_NUM_COLOR_ATTACHMENTS]             = {};
        DXGI_FORMAT         dsvFormat_                                              = DXGI_FORMAT_UNKNOWN;

        DXGI_SAMPLE_DESC    sampleDesc_                                             = { 1, 0 };

};


} // /namespace LLGL


#endif



// ================================================================================
