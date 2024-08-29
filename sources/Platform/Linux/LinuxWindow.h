/*
 * LinuxWindow.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_WINDOW_H
#define LLGL_LINUX_WINDOW_H


#include <LLGL/Window.h>
#include "LinuxDisplay.h"
#include <X11/Xlib.h>


namespace LLGL
{


class LinuxWindow : public Window
{

    public:

        #include <LLGL/Backend/Window.inl>

    public:

        LinuxWindow(const WindowDescriptor& desc);
        ~LinuxWindow();

    public:

        void ProcessEvent(XEvent& event);

    private:

        void OpenX11Window();

        void ProcessKeyEvent(XKeyEvent& event, bool down);
        void ProcessMouseKeyEvent(XButtonEvent& event, bool down);
        void ProcessExposeEvent();
        void ProcessClientMessage(XClientMessageEvent& event);
        void ProcessMotionEvent(XMotionEvent& event);

        void PostMouseKeyEvent(Key key, bool down);
        
    private:
    
        WindowDescriptor            desc_;

        LinuxSharedX11DisplaySPtr   sharedX11Display_;

        ::Display*                  display_            = nullptr;
      //::Colormap                  colorMap_;
        ::Window                    wnd_;
      //::Cursor                    cursor_;
        ::XVisualInfo*              visual_             = nullptr;
        
        ::Atom                      closeWndAtom_;
        
        Offset2D                    prevMousePos_;

};


} // /namespace LLGL


#endif



// ================================================================================
