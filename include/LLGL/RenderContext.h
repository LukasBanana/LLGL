/*
 * RenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_CONTEXT_H
#define LLGL_RENDER_CONTEXT_H


#include "RenderTarget.h"
#include "RenderContextFlags.h"
#include "RenderSystemFlags.h"
#include "Surface.h"

#include "Buffer.h"
#include "BufferArray.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "PipelineState.h"
#include "Sampler.h"
#include "QueryHeap.h"

#include <string>
#include <memory>
#include <map>


namespace LLGL
{


class Display;

/**
\brief Render context interface.
\remarks Each render context has its own surface and back buffer (or rather swap-chain) to draw into.
\see RenderSystem::CreateRenderContext
\see CommandBuffer::BeginRenderPass
*/
class LLGL_EXPORT RenderContext : public RenderTarget
{

        LLGL_DECLARE_INTERFACE( InterfaceID::RenderContext );

    public:

        /* ----- Render Target ----- */

        /**
        \brief Returns the resolution of the current video mode.
        \see GetVideoMode
        */
        Extent2D GetResolution() const final;

        //! Returns 1, since each render context has always a single color attachment.
        std::uint32_t GetNumColorAttachments() const final;

        /**
        \brief Returns true if this render context has a depth format.
        \see GetDepthStencilFormat
        \see IsDepthFormat
        */
        bool HasDepthAttachment() const final;

        /**
        \brief Returns true if this render context has a stencil format.
        \see GetDepthStencilFormat
        \see IsStencilFormat
        */
        bool HasStencilAttachment() const final;

        /* ----- Back Buffer ----- */

        //! Swaps the back buffer with the front buffer to present it on the screen (or rather on this render context).
        virtual void Present() = 0;

        /**
        \brief Returns the color format of this render context.
        \remarks This may depend on the settings specified for the video mode.
        A common value for a render context color format is Format::BGRA8UNorm.
        \see SetVideoMode
        \see AttachmentFormatDescriptor::format
        \see Format
        */
        virtual Format GetColorFormat() const = 0;

        /**
        \brief Returns the depth-stencil format of this render context.
        \remarks This may depend on the settings specified for the video mode.
        \see SetVideoMode
        \see AttachmentFormatDescriptor::format
        \see Format
        */
        virtual Format GetDepthStencilFormat() const = 0;

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
        \remarks When the video mode is changed from fullscreen to non-fullscreen, the previous surface position is restored.
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

        //! Default constructor with no effect.
        RenderContext() = default;

        //! Constructor to initialize the render context with the specified video mode and V-sync.
        RenderContext(const VideoModeDescriptor& initialVideoMode, const VsyncDescriptor& initialVsync);

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
        \brief Sets the render context surface or creates one if 'surface' is null, and switches to fullscreen mode if enabled.
        \param[in] surface Optional shared pointer to a surface which will be used as main render target.
        If this is null, a new surface is created for this render context.
        \param[in] videoModeDesc Specifies the video mode descriptor.
        The resolution of this video mode is only used if 'surface' is null,
        otherwise the resolution is determined by the content size of the specified surface (i.e. with the Surface::GetContentSize function).
        To determine the final video mode, use the GetVideoMode function.
        \param[in] windowContext Optional pointer to a NativeContextHandle structure. This is only used for desktop platforms.
        \see WindowDescriptor::windowContext
        \see Surface::GetContentSize
        \see GetVideoMode
        \see SwitchFullscreenMode
        */
        void SetOrCreateSurface(const std::shared_ptr<Surface>& surface, VideoModeDescriptor videoModeDesc, const void* windowContext);

        /**
        \brief Shares the surface and video mode with another render context.
        \note This is only used by the renderer debug layer.
        */
        void ShareSurfaceAndConfig(RenderContext& other);

        /**
        \brief Sets the display mode for the display this surface is resident in by the parameters of the video mode descriptor.
        \return Return value of the Display::SetDisplayMode function.
        \see Display::SetDisplayMode
        \see Surface::FindResidentDisplay
        */
        bool SetDisplayMode(const VideoModeDescriptor& videoModeDesc);

        /**
        \brief Sets only the fullscreen mode  and only if the specified fullscreen mode is different to the current fullscreen setting.
        \see SetDisplayMode
        \see GetVideoMode
        */
        bool SetDisplayFullscreenMode(const VideoModeDescriptor& videoModeDesc);

    private:

        bool OnIsRenderContext() const override final;

        bool SetVideoModePrimary(const VideoModeDescriptor& videoModeDesc);

        void StoreSurfacePosition();
        void RestoreSurfacePosition();

    private:

        std::shared_ptr<Surface>    surface_;

        VideoModeDescriptor         videoModeDesc_;
        VsyncDescriptor             vsyncDesc_;

        std::unique_ptr<Offset2D>   cachedSurfacePos_;

};


} // /namespace LLGL


#endif



// ================================================================================
