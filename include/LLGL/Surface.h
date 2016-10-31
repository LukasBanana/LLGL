/*
 * Surface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SURFACE_H
#define LLGL_SURFACE_H


#include "Export.h"
#include "Types.h"
#include "RenderContextDescriptor.h"


namespace LLGL
{


/**
\brief The Surface interface is the base interface for Window and Canvas.
\remarks Surface provides the minimal required interface for a graphics rendering context,
such as the access to the native handle and the ability to dynamically recreate its surface
(which is required for multi-sampled framebuffers on a WGL context for instance).
\see Window
\see Canvas
*/
class LLGL_EXPORT Surface
{

    public:

        virtual ~Surface()
        {
        }

        /**
        \brief Returns the native surface handle.
        \remarks This must be casted to a platform specific structure:
        \code
        // Example for a custom Win32 window class
        #include <LLGL/Platform/NativeHandle.h>
        //...
        void YourWindowClass::GetNativeHandle(void* nativeHandle)
        {
            auto handle = reinterpret_cast<LLGL::NativeHandle*>(nativeHandle);
            //handle->window = 'some HWND window handle';
        }
        \endcode
        */
        virtual void GetNativeHandle(void* nativeHandle) const = 0;

        /**
        \brief Recreates the internal surface object with the current descriptor settings.
        This may invalidate the native handle previously returned by "GetNativeHandle".
        \remarks This function is mainly used by the OpenGL renderer on Win32 when a multi-sampled framebuffer is created.
        \see GetNativeHandle
        */
        virtual void Recreate() = 0;

        /**
        \brief Returns the size of the surface context (or rather the drawing area).
        \remarks For the Window interface this is equivalent of calling "Window::GetSize(true)" for instance.
        \see Window::GetSize
        */
        virtual Size GetContentSize() const = 0;

        /**
        \brief Adapts the surface to fits the needs for the specified video mode descriptor.
        \param[in,out] videoModeDesc Specifies the input and output video mode descriptor.
        \return If the video mode descriptor has been accepted with no modifications and surface has been updated the return value is true.
        Otherwise the video mode descriptor has been modified to the value the surface supports and the return value is false.
        */
        virtual bool AdaptForVideoMode(VideoModeDescriptor& videoModeDesc) = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
