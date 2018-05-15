/*
 * DbgRenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_RENDER_CONTEXT_H
#define LLGL_DBG_RENDER_CONTEXT_H


#include <LLGL/RenderContext.h>


namespace LLGL
{


class DbgBuffer;

class DbgRenderContext : public RenderContext
{

    public:

        /* ----- Common ----- */

        DbgRenderContext(RenderContext& instance);

        void Present() override;

        /* ----- Debugging members ----- */

        RenderContext& instance;

    private:

        bool OnSetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        bool OnSetVsync(const VsyncDescriptor& vsyncDesc) override;

};


} // /namespace LLGL


#endif



// ================================================================================
