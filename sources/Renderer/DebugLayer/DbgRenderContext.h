/*
 * DbgRenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DBG_RENDER_CONTEXT_H__
#define __LLGL_DBG_RENDER_CONTEXT_H__


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

        /* ----- Configuration ----- */

        void SetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        void SetVsync(const VsyncDescriptor& vsyncDesc) override;

        /* ----- Debugging members ----- */

        RenderContext& instance;

};


} // /namespace LLGL


#endif



// ================================================================================
