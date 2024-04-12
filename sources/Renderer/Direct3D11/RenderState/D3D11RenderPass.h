/*
 * D3D11RenderPass.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_RENDER_PASS_H
#define LLGL_D3D11_RENDER_PASS_H


#include <LLGL/RenderPass.h>
#include <LLGL/ForwardDecls.h>
#include <LLGL/Constants.h>
#include <cstdint>
#include <d3d11.h>


namespace LLGL
{


class D3D11RenderPass final : public RenderPass
{

    public:

        D3D11RenderPass(const RenderPassDescriptor& desc);

        // Specifies the clear flags for the depth-stencil view (DSV).
        inline UINT GetClearFlagsDSV() const
        {
            return clearFlagsDSV_;
        }

        // Returns the array of color attachment indices that are meant to be cleared when a render pass begins (value of 0xFF ends the list).
        inline const std::uint8_t* GetClearColorAttachments() const
        {
            return clearColorAttachments_;
        }

        // Returns a bitwise OR combination of D3D11_DSV_FLAG entries.
        inline UINT GetAttachmentFlagsDSV() const
        {
            return attachmentFlagsDSV_;
        }

    private:

        UINT            clearFlagsDSV_                                          = 0;
        std::uint8_t    clearColorAttachments_[LLGL_MAX_NUM_COLOR_ATTACHMENTS]  = {};
        UINT            attachmentFlagsDSV_                                     = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
