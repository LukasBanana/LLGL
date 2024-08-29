/*
 * MacOSSubviewWindow.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MACOS_SUBVIEW_WINDOW_H
#define LLGL_MACOS_SUBVIEW_WINDOW_H


#include <LLGL/Window.h>

#import <Cocoa/Cocoa.h>


namespace LLGL
{


class MacOSSubviewWindow : public Window
{

    public:

        #include <LLGL/Backend/Window.inl>

    public:

        MacOSSubviewWindow(const WindowDescriptor& desc);
        ~MacOSSubviewWindow();

        // Returns the native NSView object.
        inline NSView* GetNSView() const
        {
            return view_;
        }

    private:

        NSView* CreateNSView(const WindowDescriptor& desc);

    private:

        NSView*     view_ = nullptr;
        UTF8String  title_;

};


} // /namespace LLGL


#endif



// ================================================================================
