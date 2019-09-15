/*
 * DbgRenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_RENDER_CONTEXT_H
#define LLGL_DBG_RENDER_CONTEXT_H


#include <LLGL/RenderContext.h>


namespace LLGL
{


class DbgBuffer;

class DbgRenderContext final : public RenderContext
{

    public:

        void Present() override;

        std::uint32_t GetSamples() const override;

        Format GetColorFormat() const override;
        Format GetDepthStencilFormat() const override;

        const RenderPass* GetRenderPass() const override;

    public:

        DbgRenderContext(RenderContext& instance);

    public:

        RenderContext& instance;

    private:

        bool OnSetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        bool OnSetVsync(const VsyncDescriptor& vsyncDesc) override;

};


} // /namespace LLGL


#endif



// ================================================================================
