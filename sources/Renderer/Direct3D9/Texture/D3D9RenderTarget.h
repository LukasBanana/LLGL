/*
 * D3D9RenderTarget.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_RENDER_TARGET_H
#define LLGL_D3D9_RENDER_TARGET_H


#include <LLGL/RenderTarget.h>
#include "D3D9Texture.h"
#include "../Direct3D9.h"
#include <string>
#include <vector>


namespace LLGL
{


class D3D9RenderTarget final : public RenderTarget
{

    public:

        #include <LLGL/Backend/RenderTarget.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        D3D9RenderTarget(IDirect3DDevice9* device, const RenderTargetDescriptor& desc);

    public:

        const RenderTargetDescriptor desc;

    private:

        void BuildAttachmentArray(IDirect3DDevice9* device);

        D3D9Texture* MakeIntermediateAttachment(IDirect3DDevice9* device, const Format format, std::uint32_t samples = 1);

    private:

        std::string                                 label_;
        std::vector<D3D9Texture*>                   colorAttachments_;
        std::vector<D3D9Texture*>                   resolveAttachments_;
        D3D9Texture*                                depthStencilAttachment_     = nullptr;
        Format                                      depthStencilFormat_         = Format::Undefined;
        std::vector<std::unique_ptr<D3D9Texture>>   intermediateAttachments_;

};


} // /namespace LLGL


#endif



// ================================================================================
