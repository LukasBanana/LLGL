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
#include <d3d12.h>


namespace LLGL
{


class D3D12RenderPass final : public RenderPass
{

    public:

        D3D12RenderPass(const RenderPassDescriptor& desc);

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

    private:

        UINT            numColorAttachments_                                    = 0;
        UINT            clearFlagsDSV_                                          = 0;
        std::uint8_t    clearColorAttachments_[LLGL_MAX_NUM_COLOR_ATTACHMENTS]  = {};

};


} // /namespace LLGL


#endif



// ================================================================================
