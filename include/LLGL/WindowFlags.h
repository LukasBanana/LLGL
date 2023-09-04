/*
 * WindowFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WINDOW_FLAGS_H
#define LLGL_WINDOW_FLAGS_H


#include <LLGL/Types.h>
#include <LLGL/Constants.h>
#include <LLGL/Container/Strings.h>
#include <cstdint>


namespace LLGL
{


/**
\brief Window creation flags.
\see WindowDescriptor::flags
*/
struct WindowFlags
{
    enum
    {
        //! Specifies whether the window is visible at creation time.
        Visible         = (1 << 0),

        //! Specifies whether the window is borderless. This is required for a fullscreen swap-chain.
        Borderless      = (1 << 1),

        /**
        \brief Specifies whether the window can be resized.
        \remarks For every window representing the surface for a SwapChain which has been resized,
        the video mode of that SwapChain must be updated with the resolution of the surface's content size.
        This can be done by resizing the swap-chain buffers to the new resolution before the respective swap-chain is bound to a render pass,
        or it can be handled by a window event listener inside a custom \c OnResize callback:
        \code
        // Alternative 1
        class MyEventListener : public LLGL::Window::EventListener {
            void OnResize(LLGL::Window& sender, const LLGL::Extent2D& clientAreaSize) override {
                mySwapChain->ResizeBuffers(clientAreaSize);
            }
        };
        myWindow->AddEventListener(std::make_shared<MyEventListener>());

        // Alternative 2
        mySwapChain->ResizeBuffers(myWindow->GetSize());
        myCmdBuffer->BeginRenderPass(*mySwapChain);
        \endcode
        \note Not updating the swap-chain on a resized window is undefined behavior.
        \see Surface::GetSize
        \see Window::EventListener::OnResize
        */
        Resizable       = (1 << 2),

        /**
        \brief Specifies whether the window is centered within the desktop screen at creation time.
        \remarks If this is specifies, the \c position field of the WindowDescriptor will be ignored.
        */
        Centered        = (1 << 4),

        /**
        \brief Specifies whether the window allows that files can be draged-and-droped onto the window.
        \note Only supported on: MS/Windows.
        */
        AcceptDropFiles = (1 << 3),
    };
};

/**
\brief Window descriptor structure.
\see Window::Create
*/
struct WindowDescriptor
{
    //! Window title in UTF-8 encoding.
    UTF8String      title;

    //! Window position (relative to the client area).
    Offset2D        position;

    //! Specifies the content size, i.e. not including the frame and caption dimensions.
    Extent2D        size;

    /**
    \brief Specifies the window creation flags. This can be a bitwise OR combination of the WindowFlags entries.
    \see WindowFlags
    */
    long            flags               = 0;

    /**
    \brief Window context handle.
    \remarks If used, this must be casted from a platform specific structure:
    \code
    #include <LLGL/Platform/NativeHandle.h>
    //...
    LLGL::NativeHandle myParentWindowHandle;
    myParentWindow->GetNativeHandle(&myParentWindowHandle, sizeof(myParentWindowHandle));
    windowDesc.windowContext        = &myParentWindowHandle;
    windowDesc.windowContextSize    = sizeof(myParentWindowHandle);
    \endcode
    */
    const void*     windowContext       = nullptr;

    /**
    \brief Specifies the size (in bytes) of the data type windowContext points to.
    \remarks If windowContext is non-null, this must be equal to <code>sizeof(LLGL::NativeHandle)</code>.
    \see windowContext
    */
    std::size_t     windowContextSize   = 0;
};

/**
\brief Window behavior structure.
\see Window::SetBehavior
*/
struct WindowBehavior
{
    /**
    \brief Specifies whether to clear the content of the window when it is resized. By default false.
    \remarks This is used by Win32 to erase (\c WM_ERASEBKGND message) or keep the background on a window resize.
    If this is false, some kind of flickering during a window resize can be avoided.
    \note Only supported on: Win32.
    */
    bool            disableClearOnResize    = false;

    /**
    \brief Specifies an ID for a timer which will be activated when the window is moved or sized. By default invalidWindowTimerID.
    \remarks This is used by Win32 to set a timer during a window is moved or resized to make continous scene updates.
    Do not reset it during the 'OnTimer' event, otherwise a timer might be not be released correctly!
    \note Only supported on: Win32.
    \see Window::EventListener::OnTimer
    \see invalidWindowTimerID
    */
    std::uint32_t   moveAndResizeTimerID    = Constants::invalidTimerID;
};


} // /namespace LLGL


#endif



// ================================================================================
