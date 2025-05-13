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
#include <LLGL/STL/String.h>
#include <LLGL/STL/Vector.h>


namespace LLGL
{


class NullRenderTarget final : public RenderTarget
{

    public:

        #include <LLGL/Backend/RenderTarget.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        NullRenderTarget(const RenderTargetDescriptor& desc);

    public:

        const RenderTargetDescriptor desc;

    private:

        void BuildAttachmentArray();

        NullTexture* MakeIntermediateAttachment(const Format format, std::uint32_t samples = 1);

    private:

        STL::string                                 label_;
        STL::vector<NullTexture*>                   colorAttachments_;
        STL::vector<NullTexture*>                   resolveAttachments_;
        NullTexture*                           depthStencilAttachment_     = nullptr;
        Format                                 depthStencilFormat_         = Format::Undefined;
        STL::vector<std::unique_ptr<NullTexture>>   intermediateAttachments_;

};


} // /namespace LLGL


#endif



// ================================================================================
