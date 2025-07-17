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
struct xdg_wm_base;
struct wp_viewporter;
struct zxdg_toplevel_decoration_v1;
struct zxdg_decoration_manager_v1;

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
                struct wl_surface*   surface = nullptr;
            } wl;

            struct {
                struct xdg_toplevel* toplevel = nullptr;
                struct xdg_surface*  surface  = nullptr;
                struct zxdg_toplevel_decoration_v1* decoration = nullptr;
                int decorationMode;
            } xdg;

            struct {
                struct libdecor_frame* frame = nullptr;
            } libdecor;

            struct {
                struct wl_buffer*           buffer = nullptr;
                struct wl_surface*          focus = nullptr;
                FallbackEdge                top, left, right, bottom;
                bool                        decorations = false;
            } fallback;

            float framebufferScale = 1.0f;

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

class LinuxWaylandContext
{

    public:

        static void Add(LinuxWindowWayland* window);
        static void Remove(LinuxWindowWayland* window);
        static const std::vector<LinuxWindowWayland*>& GetWindows();

    private:

        static LinuxWaylandContext& Get();

    private:

        std::vector<LinuxWindowWayland*> windows_;

};

struct WaylandState
{
    struct wl_display* display;
    struct wl_registry* registry;
    struct wl_compositor* compositor;
    struct wl_subcompositor* subcompositor;
    struct wl_seat* seat;
    struct wp_viewporter* viewporter;
    struct wl_shm* shm;

    struct zxdg_decoration_manager_v1* decorationManager;

    struct wl_pointer* pointer;
    LinuxWindowWayland* pointerFocus;
    uint32_t serial;
    uint32_t pointerEnterSerial;

    struct wl_keyboard* keyboard;
    LinuxWindowWayland* keyboardFocus;

    struct xdg_wm_base *xdg_wm_base;

    const char* tag;

    int keyRepeatTimerfd;
    int keyRepeatRate;
    int keyRepeatDelay;
    int keyRepeatScancode;

    Key keycodes[256];

    struct {
        void* handle;
        struct xkb_context*          context;
        struct xkb_keymap*           keymap;
        struct xkb_state*            state;

        struct xkb_compose_state*    composeState;

        xkb_mod_index_t              controlIndex;
        xkb_mod_index_t              altIndex;
        xkb_mod_index_t              shiftIndex;
        xkb_mod_index_t              superIndex;
        xkb_mod_index_t              capsLockIndex;
        xkb_mod_index_t              numLockIndex;
        unsigned int                 modifiers;
    } xkb;

    struct {
        struct libdecor* context;
        struct wl_callback* callback;
        bool ready = false;
    } libdecor;

    bool initialized = false;
};

extern WaylandState g_waylandState;

void HandleWaylandEvents(double* timeout);

} // /namespace LLGL

#endif // LLGL_LINUX_ENABLE_WAYLAND

#endif // LLGL_LINUX_WINDOW_WAYLAND_H



// ================================================================================
