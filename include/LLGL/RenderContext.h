/*
 * RenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_CONTEXT_H
#define LLGL_RENDER_CONTEXT_H


#include "Export.h"
#include "Surface.h"
#include "RenderContextDescriptor.h"
#include "RenderContextFlags.h"
#include "RenderSystemFlags.h"

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
\remarks Each render context has its own surface and back buffer (or rather swap-chain) to draw into.
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

        //! Returns the surface which is used to present the content on the screen.
        inline Surface& GetSurface() const
        {
            return *surface_;
        }

        /* ----- Configuration ----- */

        /**
        \brief Sets the new video mode for this render context.
        \remarks This may invalidate the currently set render target if the back buffer is required,
        so a subsequent call to "CommandBuffer::SetRenderTarget" is necessary!
        \see CommandBuffer::SetRenderTarget(RenderContext&)
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

        /**
        \brief Sets the render context surface or creates one if 'surface' is null.
        \param[in] surface Optional shared pointer to a surface which will be used as main render target.
        If this is null, a new surface is created for this render context.
        \param[in,out] videoModeDesc Specifies the video mode descriptor. This is used for reading only if 'surface' is null,
        otherwise it is used for writing only and the 'resolution' field will be set to the size of the specified surface.
        \param[in] Optional pointer to a NativeContextHandle structure. This is only used for desktop platforms.
        \see WindowDescriptor::windowContext
        */
        void SetOrCreateSurface(const std::shared_ptr<Surface>& surface, VideoModeDescriptor& videoModeDesc, const void* windowContext);

        /**
        \brief Shares the surface and video mode with another render context.
        \note This is only used by the renderer debug layer.
        */
        void ShareSurfaceAndVideoMode(RenderContext& other);

    private:

        std::shared_ptr<Surface>    surface_;
        VideoModeDescriptor         videoModeDesc_;

};


} // /namespace LLGL


#endif



// ================================================================================
