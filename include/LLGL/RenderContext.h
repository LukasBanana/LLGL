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
        \see ResizeBuffers
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
        \see AttachmentFormatDescriptor::format
        \see Format
        */
        virtual Format GetColorFormat() const = 0;

        /**
        \brief Returns the depth-stencil format of this render context.
        \remarks This may depend on the settings specified for the video mode.
        \see AttachmentFormatDescriptor::format
        \see Format
        */
        virtual Format GetDepthStencilFormat() const = 0;

        /**
        \brief Resizes all swap buffers of this swap-chain.
        \param[in] resolution Specifies the new resolution.
        \param[in] flags Optional flags to specify whether the swap-chain's surface is to be adjusted as well and to toggle fullscreen mode.
        \see GetResolution
        \see ResizeBuffersFlags
        */
        bool ResizeBuffers(const Extent2D& resolution, long flags = 0);

        /**
        \brief Sets the new vertical sychronization (V-sync) interval for this swap chain.
        \param[in] vsyncInterval Specifies the new V-sync interface.
        \return True on success, otherwise the V-sync value is invalid for this swap chain.
        \remarks This is typically 0 to disable V-sync or 1 to enable V-sync, but higher values are possible, too.
        A value of 2 for instance effectively halves the frame refresh rate that the active display is capable of,
        e.g. a display with a refresh rate of 60 Hz and a V-sync value of 2 limits the frame rate to 30 Hz.
        */
        virtual bool SetVsyncInterval(std::uint32_t vsyncInterval) = 0;

        /* ----- Surface & Display ----- */

        /**
        \brief Puts the display, the swap-chain's surface is resident in, into fullscreen mode or puts it back into normal mode.
        \param[in] enable If true, puts the display into fullscreen mode (Display::SetDisplayMode). Otherwise, puts the display back into normal mode (Display::ResetDisplayMode).
        \return True on success, otherwise the display does not support the resolution of this swap-chain.
        \remarks When switchting back from fullscreen into normal mode, this function restores the previous position of the swap-chain's surface.
        \see Display::SetDisplayMode
        \see Display::ResetDisplayMode
        */
        bool SwitchFullscreen(bool enable);

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

    protected:

        //! Default constructor with no effect.
        RenderContext() = default;

        //! Constructor to initialize the render context with the specified video mode and V-sync.
        RenderContext(const SwapChainDescriptor& desc);

        /**
        \brief Sets the render context surface or creates one if 'surface' is null, and switches to fullscreen mode if enabled.
        \param[in] surface Optional shared pointer to a surface which will be used as main render target.
        If this is null, a new surface is created for this render context.
        \param[in] size Specifies the surface content size. This is only used if \c surface is null.
        Otherwise, the size is determined by the content size of the specified surface (i.e. with the Surface::GetContentSize function).
        \param[in] fullscreen Specifies whether to put the surface into fullscreen mode.
        \param[in] windowContext Optional pointer to a NativeContextHandle structure. This is only used for desktop platforms.
        \see WindowDescriptor::windowContext
        \see Surface::GetContentSize
        \see SwitchFullscreenMode
        */
        void SetOrCreateSurface(const std::shared_ptr<Surface>& surface, const Extent2D& size, bool fullscreen, const void* windowContext);

        /**
        \brief Shares the surface and video mode with another render context.
        \note This is only used by the renderer debug layer.
        */
        void ShareSurfaceAndConfig(RenderContext& other);

        /**
        \brief Puts the display the swap-chain's surface is resident in into fullscreen mode.
        \see ResetDisplayFullscreenMode
        */
        bool SetDisplayFullscreenMode(const Extent2D& resolution);

        /**
        \brief Puts the display the swap-chain's surface is resident in back into normal mode.
        \see SetDisplayFullscreenMode
        */
        bool ResetDisplayFullscreenMode();

        /**
        \breif Primary function to resize all swap buffers.
        \see ResizeBuffers
        */
        virtual bool ResizeBuffersPrimary(const Extent2D& resolution) = 0;

    private:

        bool OnIsRenderContext() const override final;

        void StoreSurfacePosition();
        void RestoreSurfacePosition();

    private:

        std::shared_ptr<Surface>    surface_;

        Extent2D                    resolution_;

        Offset2D                    normalModeSurfacePos_;
        bool                        normalModeSurfacePosStored_ = false;

};


} // /namespace LLGL


#endif



// ================================================================================
