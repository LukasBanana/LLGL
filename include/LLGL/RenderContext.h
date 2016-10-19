/*
 * RenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_RENDER_CONTEXT_H__
#define __LLGL_RENDER_CONTEXT_H__


#include "Export.h"
#include "Window.h"
#include "RenderContextDescriptor.h"
#include "RenderContextFlags.h"
#include "RenderSystemFlags.h"
#include "ColorRGBA.h"

#include "Buffer.h"
#include "BufferArray.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "TextureArray.h"
#include "RenderTarget.h"
#include "GraphicsPipeline.h"
#include "ComputePipeline.h"
#include "Sampler.h"
#include "Query.h"

#include <Gauss/Vector3.h>
#include <string>
#include <map>


namespace LLGL
{


/**
\brief Render context interface.
\remarks Each render context has its own window and back buffer (or rather swap-chain) to draw into.
*/
class LLGL_EXPORT RenderContext
{

    public:

        /* ----- Common ----- */

        RenderContext(const RenderContext&) = delete;
        RenderContext& operator = (const RenderContext&) = delete;

        virtual ~RenderContext();

        //! Presents the back buffer on this render context.
        virtual void Present() = 0;

        //! Returns the window which is used to draw all content.
        inline Window& GetWindow() const
        {
            return *window_;
        }

        /* ----- Configuration ----- */

        /**
        \brief Sets the new video mode for this render context.
        \remarks This may invalidate the currently set render target if the back buffer is required,
        so a subsequent call to "CommandBuffer::SetRenderContext" is required!
        \see CommandBuffer::SetRenderContext(RenderContext&)
        */
        virtual void SetVideoMode(const VideoModeDescriptor& videoModeDesc);

        //! Sets the new vertical-sychronization (Vsync) configuration for this render context.
        virtual void SetVsync(const VsyncDescriptor& vsyncDesc) = 0;

        //! Returns the video mode for this render context.
        inline const VideoModeDescriptor& GetVideoMode() const
        {
            return videoModeDesc_;
        }

    protected:

        RenderContext() = default;

        void SetOrCreateWindow(const std::shared_ptr<Window>& window, VideoModeDescriptor& videoModeDesc, const void* windowContext);

        /**
        \brief Shares the window and video mode with another render context.
        \note This is only used by the renderer debug layer.
        */
        void ShareWindowAndVideoMode(RenderContext& other);

    private:

        std::shared_ptr<Window> window_;
        VideoModeDescriptor     videoModeDesc_;

};


} // /namespace LLGL


#endif



// ================================================================================
