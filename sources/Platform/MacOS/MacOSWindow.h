/*
 * MacOSWindow.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MACOS_WINDOW_H
#define LLGL_MACOS_WINDOW_H


#include <LLGL/Window.h>
#include "MacOSWindowDelegate.h"

#import <Cocoa/Cocoa.h>


namespace LLGL
{


class MacOSWindow : public Window
{

    public:

        #include <LLGL/Backend/Window.inl>

    public:

        MacOSWindow(const WindowDescriptor& desc);
        ~MacOSWindow();

        // Returns the native NSWindow object.
        inline NSWindow* GetNSWindow() const
        {
            return wnd_;
        }

        void ProcessEvent(NSEvent* event);

    private:

        void ProcessKeyEvent(NSEvent* event, bool down);
        void ProcessMouseKeyEvent(Key key, bool down);
        void ProcessMouseMoveEvent(NSEvent* event);
        void ProcessMouseWheelEvent(NSEvent* event);

        MacOSWindowDelegate* CreateNSWindowDelegate(const WindowDescriptor& desc);
        NSWindow* CreateNSWindow(const WindowDescriptor& desc);

    private:

        MacOSWindowDelegate*    wndDelegate_        = nullptr;
        NSWindow*               wnd_                = nullptr;
        Offset2D                prevMotionOffset_;

};


} // /namespace LLGL


#endif



// ================================================================================
