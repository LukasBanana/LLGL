/*
 * Surface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SURFACE_H
#define LLGL_SURFACE_H


#include "Interface.h"
#include "Types.h"
#include "RenderContextFlags.h"
#include "Display.h"


namespace LLGL
{


/**
\brief The Surface interface is the base interface for Window (on Desktop platforms) and Canvas (on mobile platforms).
\remarks Surface provides the minimal required interface for a graphics rendering context,
such as the access to the native handle, information about the content size (i.e. the client area size),
and the ability to adapt for a new video mode or an updated pixel format
(which is required for multi-sampled framebuffers on a WGL context for instance).
\see Window
\see Canvas
*/
class LLGL_EXPORT Surface : public Interface
{

        LLGL_DECLARE_INTERFACE( InterfaceID::Surface );

    public:

        /**
        \brief Returns the native surface handle.
        \param[out] nativeHandle Raw pointer to the platform specific structure to store the native handle. This must be of type LLGL::NativeHandle.
        \param[in] nativeHandleSize Specifies the size (in bytes) of the native handle structure for robustness. This must be <code>sizeof(LLGL::NativeHandle)</code>.
        \return True if the native handle was successfully retrieved. Otherwise, \c nativeHandleSize specifies an incompatible structure size.
        \remarks This must be casted to a platform specific structure:
        \code
        // Example for a custom Win32 window class
        #include <LLGL/Platform/NativeHandle.h>
        //...
        bool MyWindowClass::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) {
            if (nativeHandleSize == sizeof(LLGL::NativeHandle)) {
                auto handle = reinterpret_cast<LLGL::NativeHandle*>(nativeHandle);
                //handle->window = 'some HWND window handle';
                return true;
            }
            return false;
        }
        \endcode
        */
        virtual bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const = 0;

        /**
        \brief Returns the size of the surface context (or rather the drawing area).
        \remarks For the Window interface this is equivalent of calling <code>Window::GetSize(true)</code> for instance.
        \see Window::GetSize
        */
        virtual Extent2D GetContentSize() const = 0;

        /**
        \brief Adapts the surface to fits the needs for the specified video mode descriptor.
        \param[in,out] videoModeDesc Specifies the input and output video mode descriptor.
        \return If the video mode descriptor has been accepted with no modifications and this surface has been updated then the return value is true.
        Otherwise the video mode descriptor has been modified to the value this surface supports and the return value is false.
        */
        virtual bool AdaptForVideoMode(VideoModeDescriptor& videoModeDesc) = 0;

        /**
        \brief Resets the internal pixel format of the surface.
        \remarks This function is mainly used by the OpenGL renderer on Win32 when a multi-sampled framebuffer is created.
        \note This may invalidate the native handle previously returned by \c GetNativeHandle.
        \see GetNativeHandle
        */
        virtual void ResetPixelFormat() = 0;

        /**
        \brief Processes all events for this surface, i.e. input-, movement-, resize-, and other events.
        \remarks This function is only implemented by the Window and Canvas interfaces.
        \see Window::ProcessEvents
        \see Canvas::ProcessEvents
        */
        virtual bool ProcessEvents() = 0;

        /**
        \brief Instantiates the Display interface where this surface is resident in.
        \remarks A surface is considered resident in a display if more than the half of its client area is visible in that display.
        \return New instance of a Display where this surface is resident or null if there no display has been found.
        */
        virtual std::unique_ptr<Display> FindResidentDisplay() const = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
