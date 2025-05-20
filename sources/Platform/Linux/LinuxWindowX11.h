/*
 * LinuxWindowX11.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_WINDOW_X11_H
#define LLGL_LINUX_WINDOW_X11_H


#include <LLGL/Window.h>
#include "LinuxDisplayX11.h"
#include <X11/Xlib.h>


namespace LLGL
{

class LinuxWindowX11 : public Window {
    public:

        #include <LLGL/Backend/Window.inl>

    public:

        LinuxWindowX11(const WindowDescriptor& descriptor);
        ~LinuxWindowX11();

    public:

        bool ProcessEvents() override;

        void ProcessEvent(XEvent& event);

    private:

        void Open();

        void ProcessKeyEvent(XKeyEvent& event, bool down);
        void ProcessMouseKeyEvent(XButtonEvent& event, bool down);
        void ProcessExposeEvent();
        void ProcessClientMessage(XClientMessageEvent& event);
        void ProcessMotionEvent(XMotionEvent& event);

        void PostMouseKeyEvent(Key key, bool down);

    private:
        WindowDescriptor            desc_;
        Offset2D                    prevMousePos_;

        LinuxSharedX11DisplaySPtr   sharedX11Display_;

        ::Display*                  display_            = nullptr;
      //::Colormap                  colorMap_;
        ::Window                    wnd_;
      //::Cursor                    cursor_;
        ::XVisualInfo*              visual_             = nullptr;
        
        ::Atom                      closeWndAtom_;
};

} // /namespace LLGL


#endif



// ================================================================================
