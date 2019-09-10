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


namespace LLGL
{


/**
\brief The Surface interface is the base interface for Window (on Desktop platforms) and Canvas (on movile platforms).
\remarks Surface provides the minimal required interface for a graphics rendering context,
such as the access to the native handle, information about the content size (i.e. the client area size),
and the ability to adapt for a new video mode or an updated pixel format.
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
        \remarks This must be casted to a platform specific structure:
        \code
        // Example for a custom Win32 window class
        #include <LLGL/Platform/NativeHandle.h>
        //...
        void MyWindowClass::GetNativeHandle(void* nativeHandle) {
            auto handle = reinterpret_cast<LLGL::NativeHandle*>(nativeHandle);
            //handle->window = 'some HWND window handle';
        }
        \endcode
        \todo Add <code>std::size_t nativeHandleSize</code> parameter.
        */
        virtual void GetNativeHandle(void* nativeHandle) const = 0;

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

};


} // /namespace LLGL


#endif



// ================================================================================
