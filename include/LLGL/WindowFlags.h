/*
 * WindowFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_WINDOW_FLAGS_H
#define LLGL_WINDOW_FLAGS_H


#include "Types.h"
#include "Constants.h"
#include <string>
#include <cstdint>


namespace LLGL
{


//! Window descriptor structure.
struct WindowDescriptor
{
    //! Window title as unicode string.
    std::wstring    title;

    //! Window position (relative to the client area).
    Offset2D        position;

    //! Window size (this should be the client area size).
    Extent2D        size;

    //! Specifies whether the window is visible at creation time. By default false.
    bool            visible             = false;

    //! Specifies whether the window is borderless. This is required for a fullscreen render context. By default false.
    bool            borderless          = false;

    /**
    \brief Specifies whether the window can be resized. By default false.
    \remarks For every window representing the surface for a RenderContext which has been resized,
    the video mode of that RenderContext must be updated with the resolution of the surface's content size.
    This can be done by setting the video mode with the new resolution before the respective render context is set as render target,
    or it can be handled by a window event listener on the 'OnResize' callback:
    \code
    // Alternative 1
    class MyEventListener : public LLGL::Window::EventListener {
        void OnResize(Window& sender, const Extent2D& clientAreaSize) override {
            auto myVideoMode = myRenderContext->GetVideoMode();
            myVideoMode.resolution = clientAreaSize;
            myRenderContext->SetVideoMode(myVideoMode);
        }
    };
    myWindow->AddEventListener(std::make_shared<MyEventListener>());

    // Alternative 2
    auto myVideoMode = myRenderContext->GetVideoMode();
    myVideoMode.resolution = myWindow->GetContentSize();
    myRenderContext->SetVideoMode(myVideoMode);
    myCmdBuffer->SetRenderTarget(*myRenderContext);
    \endcode
    \note Not updating the render context on a resized window is undefined behavior.
    \see RenderContext::SetVideoMode
    \see Surface::GetContentSize
    \see Window::EventListener::OnResize
    */
    bool            resizable           = false;

    /**
    \brief Specifies whether the window allows that files can be draged-and-droped onto the window. By default false.
    \note Only supported on: MS/Windows.
    */
    bool            acceptDropFiles     = false;

    /**
    \brief Specifies whether this window prevents the host system for power-safe mode. By default false.
    \note Only supported on: MS/Windows.
    */
    bool            preventForPowerSafe = false;

    //! Specifies whether the window is centered within the desktop screen. By default false.
    bool            centered            = false;

    /**
    \brief Window context handle.
    \remarks If used, this must be casted from a platform specific structure:
    \code
    #include <LLGL/Platform/NativeHandle.h>
    //...
    LLGL::NativeContextHandle handle;
    //handle.parentWindow = ...
    windowDesc.windowContext = reinterpret_cast<const void*>(&handle);
    \endcode
    */
    const void*     windowContext       = nullptr;
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
