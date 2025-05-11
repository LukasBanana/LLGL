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

class LinuxX11Window : public Window {
    public:

        #include <LLGL/Backend/Window.inl>

    public:

        LinuxX11Window(const WindowDescriptor& descriptor);
        ~LinuxX11Window();

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

struct wayland_state {
    struct wl_display* display;
    struct wl_registry* registry;
    struct wl_compositor* compositor;
    struct wl_surface* surface;
    struct wl_seat* seat;

    struct wl_pointer* pointer;
    uint32_t serial;
    uint32_t pointerEnterSerial;

    struct wl_keyboard* keyboard;

    const char* tag;

    bool hovered;
};

class LinuxWaylandWindow : public Window {
    public:

        #include <LLGL/Backend/Window.inl>

    public:

        LinuxWaylandWindow(const WindowDescriptor& descriptor);
        ~LinuxWaylandWindow();

    public:

        bool ProcessEvents() override;

        // void ProcessEvent(wl_);

        wayland_state& GetWaylandState() {
            return wl_;
        }

        void ProcessKeyEvent(XKeyEvent& event, bool down);
        void ProcessMouseKeyEvent(uint32_t button, bool down);
        void ProcessExposeEvent();
        void ProcessClientMessage(XClientMessageEvent& event);
        void ProcessMotionEvent(int xpos, int ypos);

        void PostMouseKeyEvent(Key key, bool down);

    private:

        void Open();

    private:
        WindowDescriptor            desc_;
        Offset2D                    prevMousePos_;

        wayland_state wl_;
};

} // /namespace LLGL


#endif



// ================================================================================
