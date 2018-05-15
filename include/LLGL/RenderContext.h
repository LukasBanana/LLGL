/*
 * RenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_CONTEXT_H
#define LLGL_RENDER_CONTEXT_H


#include "Export.h"
#include "Surface.h"
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

#include <string>
#include <map>


namespace LLGL
{


/**
\brief Render context interface.
\remarks Each render context has its own surface and back buffer (or rather swap-chain) to draw into.
\todo Make this a sub class of RenderTarget as soon as all "Attach..." and "Detach..." functions have been removed.
*/
class LLGL_EXPORT RenderContext
{

    public:

        /* ----- Common ----- */

        RenderContext(const RenderContext&) = delete;
        RenderContext& operator = (const RenderContext&) = delete;

        virtual ~RenderContext();

        //! Swaps the back buffer with the front buffer to present it on the screen (or rather on this render context).
        virtual void Present() = 0;

        /**
        \brief Returns the surface which is used to present the content on the screen.
        \remarks On desktop platforms, this can be statically casted to 'LLGL::Window&',
        and on mobile platforms, this can be statically casted to 'LLGL::Canvas&':
        \code
        auto& myWindow = static_cast<LLGL::Window&>(myRenderContext->GetSurface());
        \endcode
        */
        inline Surface& GetSurface() const
        {
            return *surface_;
        }

        /* ----- Configuration ----- */

        /**
        \brief Sets the new video mode for this render context.
        \param[in] videoModeDesc Specifies the descriptor of the new video mode.
        \return True on success, otherwise the specified video mode was invalid (e.g. if the resolution contains a zero).
        \remarks This may invalidate the currently set render target if the back buffer is required,
        so a subsequent call to "CommandBuffer::SetRenderTarget" is necessary!
        \see CommandBuffer::SetRenderTarget(RenderContext&)
        */
        bool SetVideoMode(const VideoModeDescriptor& videoModeDesc);

        //! Returns the video mode for this render context.
        inline const VideoModeDescriptor& GetVideoMode() const
        {
            return videoModeDesc_;
        }

        /**
        \brief Sets the new vertical-sychronization (V-sync) configuration for this render context.
        \param[in] vsyncDesc Specifies the descriptor of the new V-sync configuration.
        \return True on success, otherwise the specified V-sync is invalid.
        */
        bool SetVsync(const VsyncDescriptor& vsyncDesc);

        //! Returns the V-snyc configuration for this render context.
        inline const VsyncDescriptor& GetVsync() const
        {
            return vsyncDesc_;
        }

    protected:

        RenderContext() = default;

        /**
        \brief Callback when the video mode is about to get changed.
        \param[in] videoModeDesc Specifies the descriptor of the new video mode.
        \return True on success, otherwise the previous video mode remains.
        \remarks Only if the function returns true, the new video mode will be stored in the member returned by 'GetVideoMode'.
        This callback is only called when the parameter of 'SetVideoMode' differs from the previous video mode.
        \note This video mode may differ from the one passed by 'SetVideoMode' if the platform specific 'Surface' object has modified it for its requirements.
        \see SetVideoMode
        \see GetVideoMode
        \see Surface::AdaptForVideoMode
        */
        virtual bool OnSetVideoMode(const VideoModeDescriptor& videoModeDesc) = 0;

        /**
        \brief Callback when the V-sync is about to get changed.
        \param[in] vsyncDesc Specifies the descriptor of the new V-sync configuration.
        \return True on success, otherwise the previous V-sync configuration remains.
        */
        virtual bool OnSetVsync(const VsyncDescriptor& vsyncDesc) = 0;

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
        VsyncDescriptor             vsyncDesc_;

};


} // /namespace LLGL


#endif



// ================================================================================
