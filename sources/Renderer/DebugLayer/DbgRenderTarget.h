/*
 * DbgRenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_RENDER_TARGET_H
#define LLGL_DBG_RENDER_TARGET_H


#include <LLGL/RenderTarget.h>
#include <string>


namespace LLGL
{


class RenderingDebugger;

class DbgRenderTarget final : public RenderTarget
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

        DbgRenderTarget(RenderTarget& instance, RenderingDebugger* debugger, const RenderTargetDescriptor& desc);

    public:

        RenderTarget&                   instance;
        const RenderTargetDescriptor    desc;
        std::string                     label;

};


} // /namespace LLGL


#endif



// ================================================================================
