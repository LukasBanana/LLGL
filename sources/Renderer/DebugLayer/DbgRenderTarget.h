/*
 * DbgRenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_RENDER_TARGET_H
#define LLGL_DBG_RENDER_TARGET_H


#include <LLGL/RenderTarget.h>


namespace LLGL
{


class RenderingDebugger;

class DbgRenderTarget : public RenderTarget
{

    public:

        DbgRenderTarget(RenderTarget& instance, RenderingDebugger* debugger, const RenderTargetDescriptor& desc);

        Extent2D GetResolution() const override;
        std::uint32_t GetNumColorAttachments() const override;
        bool HasDepthAttachment() const override;
        bool HasStencilAttachment() const override;

        // Returns the render target descriptor of this debug layer object.
        inline const RenderTargetDescriptor& GetDesc() const
        {
            return desc_;
        }

        RenderTarget&           instance;

    private:

        RenderTargetDescriptor  desc_;

};


} // /namespace LLGL


#endif



// ================================================================================
