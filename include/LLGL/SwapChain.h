/*
 * SwapChain.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SWAP_CHAIN_H
#define LLGL_SWAP_CHAIN_H


#include <LLGL/RenderTarget.h>
#include <LLGL/SwapChainFlags.h>
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/Surface.h>

#include <LLGL/Buffer.h>
#include <LLGL/BufferArray.h>
#include <LLGL/Texture.h>
#include <LLGL/RenderTarget.h>
#include <LLGL/PipelineState.h>
#include <LLGL/Sampler.h>
#include <LLGL/QueryHeap.h>

#include <string>
#include <memory>
#include <map>


namespace LLGL
{


class Display;

/**
\brief Swap-chain interface.
\remarks Each swap-chain has its own surface and swap buffers to draw into.
\see RenderSystem::CreateSwapChain
\see CommandBuffer::BeginRenderPass
*/
class LLGL_EXPORT SwapChain : public RenderTarget
{

        LLGL_DECLARE_INTERFACE( InterfaceID::SwapChain );

    public:

        //! Release the internal data.
        ~SwapChain();

        /* ----- Render Target ----- */

        /**
        \brief Returns the resolution of the current video mode.
        \see ResizeBuffers
        */
        Extent2D GetResolution() const final;

        //! Returns 1, since each swap-chain has always a single color attachment.
        std::uint32_t GetNumColorAttachments() const final;

        /**
        \brief Returns true if this swap-chain has a depth format.
        \see GetDepthStencilFormat
        \see IsDepthFormat
        */
        bool HasDepthAttachment() const final;

        /**
        \brief Returns true if this swap-chain has a stencil format.
        \see GetDepthStencilFormat
        \see IsStencilFormat
        */
        bool HasStencilAttachment() const final;

        /* ----- Back Buffer ----- */

        /**
        \brief Swaps the current back buffer with the front buffer to present it on the screen.
        \see GetCurrentSwapIndex
        */
        virtual void Present() = 0;

        /**
        \brief Returns the current swap-buffer index.

        \remarks If the renderer supports control over swap-chain sizes, this function returns the current swap-buffer index. Otherwise, this function always returns 0.
        \remarks This function is guaranteed to never return a value greater than or equal to the swap-chain size that was specified when this swap-chain was created.
        \remarks Can be used to encode a command buffer for a specific swap-buffer.

        \return \f$i \in \left[ 0, \texttt{GetNumSwapBuffers()} \right)\f$

        \see GetNumSwapBuffers
        \see CommandBuffer::BeginRenderPass
        */
        virtual std::uint32_t GetCurrentSwapIndex() const = 0;

        /**
        \brief Returns the actual number of swap-buffers in this swap-chain.

        \remarks This value is either 1 if the renderer does not support swap-chain size control
        or a value derived from SwapChainDescriptor::swapBuffers this swap-chain was created with.
        It is not guaranteed to be equal SwapChainDescriptor::swapBuffers even if the renderer supports swap-chain size control,
        because there are different limitations of how many swap buffers can be created.

        \return A value in the range \f$\left[ 1, \infty+ \right)\f$ but \e usually in the range \f$ \left[1, 3\right]\f$.

        \see GetCurrentSwapIndex
        \see SwapChainDescriptor::swapBuffers
        */
        virtual std::uint32_t GetNumSwapBuffers() const = 0;

        /**
        \brief Returns the color format of this swap-chain.
        \remarks This may depend on the settings specified for the video mode.
        A common value for a swap-chain color format is Format::BGRA8UNorm.
        \see AttachmentFormatDescriptor::format
        \see Format
        */
        virtual Format GetColorFormat() const = 0;

        /**
        \brief Returns the depth-stencil format of this swap-chain.
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

    public:

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
        auto& myWindow = static_cast<LLGL::Window&>(mySwapChain->GetSurface());
        \endcode
        */
        Surface& GetSurface() const;

    protected:

        /**
        \brief Primary function to resize all swap buffers.
        \see ResizeBuffers
        */
        virtual bool ResizeBuffersPrimary(const Extent2D& resolution) = 0;

    protected:

        //! Allocates the internal data.
        SwapChain();

        //! Constructor to initialize the swap-chain with the specified video mode and V-sync.
        SwapChain(const SwapChainDescriptor& desc);

        /**
        \brief Sets the swap-chain surface or creates one if 'surface' is null, and switches to fullscreen mode if enabled.
        \param[in] surface Optional shared pointer to a surface which will be used as main render target.
        If this is null, a new surface is created for this swap-chain.
        \param[in] size Specifies the surface content size. This is only used if \c surface is null.
        Otherwise, the size is determined by the content size of the specified surface (i.e. with the Surface::GetContentSize function).
        \param[in] fullscreen Specifies whether to put the surface into fullscreen mode.
        \param[in] windowContext Optional pointer to a NativeHandle structure. This is only used for desktop platforms.
        \see WindowDescriptor::windowContext
        \see Surface::GetContentSize
        \see SwitchFullscreenMode
        */
        void SetOrCreateSurface(
            const std::shared_ptr<Surface>& surface,
            const Extent2D&                 size,
            bool                            fullscreen,
            const void*                     windowContext       = nullptr,
            std::size_t                     windowContextSize   = 0
        );

        /**
        \brief Shares the surface and resolution with another swap-chain.
        \note This is only used by the renderer debug layer.
        */
        void ShareSurfaceAndConfig(SwapChain& other);

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

    private:

        void StoreSurfacePosition();
        void RestoreSurfacePosition();

    private:

        struct Pimpl;
        Pimpl* pimpl_;

};


} // /namespace LLGL


#endif



// ================================================================================
