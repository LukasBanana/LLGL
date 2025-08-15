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

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <libdecor-0/libdecor.h>

struct xdg_toplevel;
struct xdg_surface;
struct zxdg_toplevel_decoration_v1;

struct FallbackEdge
{
    struct wl_surface*          surface = nullptr;
    struct wl_subsurface*       subsurface = nullptr;
    struct wp_viewport*         viewport = nullptr;
};

namespace LLGL
{

class LinuxWindowWayland : public Window {
    public:

        #include <LLGL/Backend/Window.inl>

    public:

        LinuxWindowWayland(const WindowDescriptor& descriptor);
        ~LinuxWindowWayland();

    public:

        void ProcessEvents();

        void ProcessKeyEvent(Key event, bool down);
        void ProcessMouseKeyEvent(uint32_t button, bool down);
        void ProcessWheelMotionEvent(int motion);
        void ProcessExposeEvent();
        void ProcessMotionEvent(int xpos, int ypos);
        void ProcessFocusEvent(bool focused);

        void SetSizeInternal(Extent2D size);

    public:

        struct State {
            Offset2D                    prevMousePos;
            Offset2D                    position;
            Extent2D                    size;
            Extent2D                    framebufferSize;

            struct {
                wl_surface*   surface = nullptr;
            } wl;

            struct {
                xdg_toplevel* toplevel = nullptr;
                xdg_surface*  surface  = nullptr;
                zxdg_toplevel_decoration_v1* decoration = nullptr;
                int decorationMode;
            } xdg;

            struct {
                libdecor_frame* frame = nullptr;
            } libdecor;

            struct {
                wl_buffer*           buffer = nullptr;
                wl_surface*          focus = nullptr;
                FallbackEdge         top;
                FallbackEdge         left;
                FallbackEdge         right;
                FallbackEdge         bottom;
                bool                 decorations = false;
            } fallback;

            wl_output*                  monitor = nullptr;

            float                       framebufferScale = 1.0f;

            bool                        hovered     = false;
            bool                        shouldClose = false;
            bool                        visible     = false;
            bool                        maximized   = false;
            bool                        activated   = false;
            bool                        fullscreen  = false;
            bool                        resizable   = true;
            bool                        decorated   = true;
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
