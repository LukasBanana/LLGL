/*
 * WGRenderPass.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_RENDER_PASS_H
#define LLGL_WG_RENDER_PASS_H


#include <LLGL/RenderPass.h>
#include <LLGL/RenderPassFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/SmallVector.h>
#include <webgpu/webgpu.h>


namespace LLGL
{


class WGRenderPass final : public RenderPass
{

    public:

        WGRenderPass(const RenderPassDescriptor& desc);

        // Returns the WebGPU color target formats.
        inline ArrayView<WGPUTextureFormat> GetColorTargetFormats() const
        {
            return colorTargetFormats_;
        }

    private:

        SmallVector<WGPUTextureFormat, 1> colorTargetFormats_;

};


} // /namespace LLGL


#endif



// ================================================================================
