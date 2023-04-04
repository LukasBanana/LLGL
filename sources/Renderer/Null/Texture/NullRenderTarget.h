/*
 * NullRenderTarget.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_NULL_RENDER_TARGET_H
#define LLGL_NULL_RENDER_TARGET_H


#include <LLGL/RenderTarget.h>
#include "NullTexture.h"
#include <string>
#include <vector>


namespace LLGL
{


class NullRenderTarget final : public RenderTarget
{

    public:

        void SetName(const char* name) override;

        Extent2D GetResolution() const override;
        std::uint32_t GetSamples() const override;
        std::uint32_t GetNumColorAttachments() const override;

        bool HasDepthAttachment() const override;
        bool HasStencilAttachment() const override;

        const RenderPass* GetRenderPass() const override;

    public:

        NullRenderTarget(const RenderTargetDescriptor& desc);

    public:

        const RenderTargetDescriptor desc;

    private:

        void BuildAttachmentArray();

        NullTexture* MakeIntermediateAttachment(const AttachmentDescriptor& attachmentDesc);

    private:

        std::string                                 label_;
        std::vector<NullTexture*>                   colorAttachments_;
        NullTexture*                                depthStencilAttachment_     = nullptr;
        std::vector<std::unique_ptr<NullTexture>>   intermediateAttachments_;
        Format                                      depthStencilFormat_         = Format::Undefined;

};


} // /namespace LLGL


#endif



// ================================================================================
