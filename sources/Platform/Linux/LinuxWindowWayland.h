/*
 * LinuxWindowWayland.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_WINDOW_WAYLAND_H
#define LLGL_LINUX_WINDOW_WAYLAND_H

#if LLGL_LINUX_ENABLE_WAYLAND

#include <LLGL/Window.h>

#include "protocols/xdg-shell-client-protocol.h"


namespace LLGL
{

class LinuxWindowWayland : public Window {
    public:

        #include <LLGL/Backend/Window.inl>

    public:

        LinuxWindowWayland(const WindowDescriptor& descriptor);
        ~LinuxWindowWayland();

    public:

        bool ProcessEvents() override;

        void ProcessMouseKeyEvent(uint32_t button, bool down);
        void ProcessWheelMotionEvent(int motion);
        void ProcessExposeEvent();
        void ProcessMotionEvent(int xpos, int ypos);

        void PostMouseKeyEvent(Key key, bool down);

        void SetSizeInternal(Extent2D size);

    public:

        struct State {
            Offset2D                    prevMousePos;
            Offset2D                    position;
            Extent2D                    size;
            Extent2D                    prevSize;
            UTF8String                  title;

            struct wl_surface*          wl_surface = nullptr;
            struct xdg_toplevel*        xdg_toplevel = nullptr;
            struct xdg_surface*         xdg_surface = nullptr;

            bool                        hovered = false;
            bool                        closed  = false;
        };

        State& GetState();

    private:

        void Open();

    private:
        WindowDescriptor            desc_;
        State                       state_;
};

} // /namespace LLGL

#endif // LLGL_LINUX_ENABLE_WAYLAND

#endif // LLGL_LINUX_WINDOW_WAYLAND_H



// ================================================================================
