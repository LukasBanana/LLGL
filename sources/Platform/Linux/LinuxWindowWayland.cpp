/*
 * LinuxWindowWayland.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#if LLGL_LINUX_ENABLE_WAYLAND

#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Display.h>
#include <LLGL/Timer.h>
#include <LLGL/Log.h>

#include "LinuxWindowWayland.h"
#include "../../Core/Exception.h"
#include "../../Core/Assertion.h"

#include <unistd.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
#include <poll.h>
#include <fcntl.h>
#include <wayland-client.h>
#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>

#include "protocols/xdg-shell-client-protocol.h"
#include "protocols/xdg-decoration-client-protocol.h"
#include "protocols/viewporter-client-protocol.h"

namespace LLGL
{

struct Image {
    int width;
    int height;
    uint8_t* pixels;
};

static constexpr int DECORATION_BORDER_SIZE    = 4;
static constexpr int DECORATION_CAPTION_HEIGHT = 24;

WaylandState g_waylandState = {};

LinuxWaylandContext& LinuxWaylandContext::Get()
{
    static LinuxWaylandContext instance;
    return instance;
}

void LinuxWaylandContext::Add(LinuxWindowWayland* window)
{
    LinuxWaylandContext& context = LinuxWaylandContext::Get();
    context.windows_.push_back(window);
}

void LinuxWaylandContext::Remove(LinuxWindowWayland* window)
{
    LinuxWaylandContext& context = LinuxWaylandContext::Get();

    for (auto it = context.windows_.begin(); it != context.windows_.end(); ++it)
    {
        if ((*it) == window)
        {
            context.windows_.erase(it);
            break;
        }
    }
}

const std::vector<LinuxWindowWayland*>& LinuxWaylandContext::GetWindows()
{
    LinuxWaylandContext& context = LinuxWaylandContext::Get();
    return context.windows_;
}

// ================================
// ======== SURFACE EVENTS ========
// ================================

void SurfaceHandleEnter(void* userData, struct wl_surface* surface, struct wl_output* output)
{

}

void SurfaceHandleLeave(void* userData, struct wl_surface* surface, struct wl_output* output)
{

}

const static wl_surface_listener SURFACE_LISTENER = {
    SurfaceHandleEnter,
    SurfaceHandleLeave
};

// ================================
// ======== POINTER EVENTS ========
// ================================

void PointerHandleEnter(
    void*               userData,
    struct wl_pointer*  pointer,
    uint32_t            serial,
    struct wl_surface*  surface,
    wl_fixed_t          surface_x,
    wl_fixed_t          surface_y)
{
    if (!surface)
        return;

    if (wl_proxy_get_tag(reinterpret_cast<struct wl_proxy*>(surface)) != &g_waylandState.tag)
        return;

    LinuxWindowWayland* window = static_cast<LinuxWindowWayland*>(wl_surface_get_user_data(surface));
    if (!window)
        return;

    g_waylandState.serial = serial;
    g_waylandState.pointerEnterSerial = serial;
    g_waylandState.pointerFocus = window;

    LinuxWindowWayland::State& state = window->GetState();

    if (surface == state.wl.surface)
    {
        state.hovered = true;
    }
}

void PointerHandleLeave(
    void*              userData,
    struct wl_pointer* pointer,
    uint32_t           serial,
    struct wl_surface* surface)
{
    if (!surface)
        return;

    LinuxWindowWayland* window = g_waylandState.pointerFocus;
    if (!window)
        return;

    if (wl_proxy_get_tag(reinterpret_cast<struct wl_proxy*>(surface)) != &g_waylandState.tag)
        return;

    g_waylandState.serial = serial;
    g_waylandState.pointerFocus = nullptr;

    LinuxWindowWayland::State& state = window->GetState();

    if (state.hovered)
    {
        state.hovered = false;
    }
}

void PointerHandleMotion(
    void*               userData,
    struct wl_pointer*  pointer,
    uint32_t            time,
    wl_fixed_t          surface_x,
    wl_fixed_t          surface_y)
{
    LinuxWindowWayland* window = g_waylandState.pointerFocus;
    if (!window)
        return;

    LinuxWindowWayland::State& state = window->GetState();

    if (!state.hovered)
        return;

    const int xpos = wl_fixed_to_int(surface_x);
    const int ypos = wl_fixed_to_int(surface_y);

    window->ProcessMotionEvent(xpos, ypos);
}

void PointerHandleButton(
    void*               userData,
    struct wl_pointer*  pointer,
    uint32_t            serial,
    uint32_t            time,
    uint32_t            button,
    uint32_t            state)
{
    LinuxWindowWayland* window = g_waylandState.pointerFocus;
    if (!window)
        return;

    LinuxWindowWayland::State& windowState = window->GetState();

    if (!windowState.hovered)
        return;

    g_waylandState.serial = serial;

    window->ProcessMouseKeyEvent(button, state == WL_POINTER_BUTTON_STATE_PRESSED);
}

void PointerHandleAxis(
    void*               userData,
    struct wl_pointer*  pointer,
    uint32_t            time,
    uint32_t            axis,
    wl_fixed_t          value)
{
    LinuxWindowWayland* window = g_waylandState.pointerFocus;
    if (!window)
        return;

    // TODO: Handle horizontal scroll?
    if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
    {
        int motion = static_cast<int>(-wl_fixed_to_double(value) / 10.0);
        window->ProcessWheelMotionEvent(motion);
    }
}

const static wl_pointer_listener POINTER_LISTENER = {
    PointerHandleEnter,
    PointerHandleLeave,
    PointerHandleMotion,
    PointerHandleButton,
    PointerHandleAxis
};

// ===============================
// ======= KEYBOARD EVENTS =======
// ===============================

static void KeyboardHandleKeymap(
    void*               userData,
    struct wl_keyboard* keyboard,
    uint32_t            format,
    int                 fd,
    uint32_t            size)
{
    struct xkb_keymap* keymap;
    struct xkb_state* state;
    struct xkb_compose_table* composeTable;
    struct xkb_compose_state* composeState;

    char* mapStr;
    const char* locale;

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
    {
        close(fd);
        return;
    }

    mapStr = static_cast<char*>(mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0));
    if (mapStr == MAP_FAILED)
    {
        close(fd);
        return;
    }

    keymap = xkb_keymap_new_from_string(g_waylandState.xkb.context,
                                        mapStr,
                                        XKB_KEYMAP_FORMAT_TEXT_V1,
                                        XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(mapStr, size);
    close(fd);

    if (!keymap)
        LLGL_TRAP("Failed to compile keymap");

    state = xkb_state_new(keymap);
    if (!state)
    {
        xkb_keymap_unref(keymap);
        LLGL_TRAP("Failed to create XKB state");
    }

    // Look up the preferred locale, falling back to "C" as default.
    locale = getenv("LC_ALL");
    if (!locale)
        locale = getenv("LC_CTYPE");
    if (!locale)
        locale = getenv("LANG");
    if (!locale)
        locale = "C";

    composeTable = xkb_compose_table_new_from_locale(g_waylandState.xkb.context, locale, XKB_COMPOSE_COMPILE_NO_FLAGS);
    if (composeTable)
    {
        composeState = xkb_compose_state_new(composeTable, XKB_COMPOSE_STATE_NO_FLAGS);
        xkb_compose_table_unref(composeTable);
        if (composeState)
            g_waylandState.xkb.composeState = composeState;
        else
            LLGL_TRAP("Failed to create XKB compose state");
    }
    else
    {
        LLGL_TRAP("Failed to create XKB compose table");
    }

    xkb_keymap_unref(g_waylandState.xkb.keymap);
    xkb_state_unref(g_waylandState.xkb.state);
    g_waylandState.xkb.keymap = keymap;
    g_waylandState.xkb.state = state;

    g_waylandState.xkb.controlIndex  = xkb_keymap_mod_get_index(g_waylandState.xkb.keymap, "Control");
    g_waylandState.xkb.altIndex      = xkb_keymap_mod_get_index(g_waylandState.xkb.keymap, "Mod1");
    g_waylandState.xkb.shiftIndex    = xkb_keymap_mod_get_index(g_waylandState.xkb.keymap, "Shift");
    g_waylandState.xkb.superIndex    = xkb_keymap_mod_get_index(g_waylandState.xkb.keymap, "Mod4");
    g_waylandState.xkb.capsLockIndex = xkb_keymap_mod_get_index(g_waylandState.xkb.keymap, "Lock");
    g_waylandState.xkb.numLockIndex  = xkb_keymap_mod_get_index(g_waylandState.xkb.keymap, "Mod2");
}

static void KeyboardHandleEnter(
    void*               userData,
    struct wl_keyboard* keyboard,
    uint32_t            serial,
    struct wl_surface*  surface,
    struct wl_array*    keys)
{
    // Happens in the case we just destroyed the surface.
    if (!surface)
        return;

    if (wl_proxy_get_tag((struct wl_proxy*) surface) != &g_waylandState.tag)
        return;

    LinuxWindowWayland* window = static_cast<LinuxWindowWayland*>(wl_surface_get_user_data(surface));
    if (!window)
        return;

    if (surface != window->GetState().wl.surface)
        return;

    g_waylandState.serial = serial;
    g_waylandState.keyboardFocus = window;

    window->ProcessFocusEvent(true);
}

static void KeyboardHandleLeave(void* userData,
                                struct wl_keyboard* keyboard,
                                uint32_t serial,
                                struct wl_surface* surface)
{
    LinuxWindowWayland* window = static_cast<LinuxWindowWayland*>(userData);

    if (!window)
        return;

    g_waylandState.serial = serial;
    g_waylandState.keyboardFocus = nullptr;

    window->ProcessFocusEvent(false);
}

static Key translateKey(uint32_t scancode)
{
    if (scancode < sizeof(g_waylandState.keycodes) / sizeof(g_waylandState.keycodes[0]))
        return g_waylandState.keycodes[scancode];

    return Key::Any;
}

static void KeyboardHandleKey(void* userData,
                              struct wl_keyboard* keyboard,
                              uint32_t serial,
                              uint32_t time,
                              uint32_t scancode,
                              uint32_t state)
{
    LinuxWindowWayland* window = g_waylandState.keyboardFocus;
    if (!window)
        return;

    g_waylandState.serial = serial;

    struct itimerspec timer = {0};

    const bool down = state == WL_KEYBOARD_KEY_STATE_PRESSED;

    if (down)
    {
        const xkb_keycode_t keycode = scancode + 8;

        if (xkb_keymap_key_repeats(g_waylandState.xkb.keymap, keycode) && g_waylandState.keyRepeatRate > 0)
        {
            g_waylandState.keyRepeatScancode = scancode;
            if (g_waylandState.keyRepeatRate > 1)
                timer.it_interval.tv_nsec = 1000000000 / g_waylandState.keyRepeatRate;
            else
                timer.it_interval.tv_sec = 1;

            timer.it_value.tv_sec = g_waylandState.keyRepeatDelay / 1000;
            timer.it_value.tv_nsec = (g_waylandState.keyRepeatDelay % 1000) * 1000000;
        }
    }

    const Key key = translateKey(scancode);

    window->ProcessKeyEvent(key, down);
}

static void KeyboardHandleModifiers(void* userData,
                                    struct wl_keyboard* keyboard,
                                    uint32_t serial,
                                    uint32_t modsDepressed,
                                    uint32_t modsLatched,
                                    uint32_t modsLocked,
                                    uint32_t group)
{
    g_waylandState.serial = serial;

    if (!g_waylandState.xkb.keymap)
        return;

    xkb_state_update_mask(g_waylandState.xkb.state, modsDepressed, modsLatched, modsLocked, 0, 0, group);

    // TODO: LLGL doesn't support modifiers?

    // g_waylandState.xkb.modifiers = 0;

    // struct
    // {
    //     xkb_mod_index_t index;
    //     LLGL::Key bit;
    // } modifiers[] =
    // {
    //     { g_waylandState.xkb.controlIndex,  Key::Control },
    //     // { g_waylandState.xkb.altIndex,      Key::Alt },
    //     { g_waylandState.xkb.shiftIndex,    Key::Shift },
    //     // { g_waylandState.xkb.superIndex,    Key::Win },
    //     // { g_waylandState.xkb.capsLockIndex, Key::CapsLock },
    //     { g_waylandState.xkb.numLockIndex,  Key::NumLock }
    // };

    // for (size_t i = 0; i < sizeof(modifiers) / sizeof(modifiers[0]); i++)
    // {
    //     if (xkb_state_mod_index_is_active(g_waylandState.xkb.state,
    //                                       modifiers[i].index,
    //                                       XKB_STATE_MODS_EFFECTIVE) == 1)
    //     {
    //         g_waylandState.xkb.modifiers |= modifiers[i].bit;
    //     }
    // }
}

static void KeyboardHandleRepeatInfo(void* userData,
                                     struct wl_keyboard* keyboard,
                                     int32_t rate,
                                     int32_t delay)
{
    if (keyboard != g_waylandState.keyboard)
        return;

    g_waylandState.keyRepeatRate = rate;
    g_waylandState.keyRepeatDelay = delay;
}

const static wl_keyboard_listener keyboardListener = {
    KeyboardHandleKeymap,
    KeyboardHandleEnter,
    KeyboardHandleLeave,
    KeyboardHandleKey,
    KeyboardHandleModifiers,
    KeyboardHandleRepeatInfo
};

// ===============================
// ========= SEAT EVENTS =========
// ===============================

void SeatHandleCapabilities(void* userData, struct wl_seat* seat, uint32_t caps)
{
    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !g_waylandState.pointer)
    {
        g_waylandState.pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(g_waylandState.pointer, &POINTER_LISTENER, nullptr);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && g_waylandState.pointer)
    {
        wl_pointer_destroy(g_waylandState.pointer);
        g_waylandState.pointer = nullptr;
    }

    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !g_waylandState.keyboard)
    {
        g_waylandState.keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(g_waylandState.keyboard, &keyboardListener, nullptr);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && g_waylandState.keyboard)
    {
        wl_keyboard_destroy(g_waylandState.keyboard);
        g_waylandState.keyboard = nullptr;
    }
}

void SeatHandleName(void* userData, struct wl_seat* seat, const char* name)
{
}

const static struct wl_seat_listener SEAT_LISTENER = {
    SeatHandleCapabilities,
    SeatHandleName
};

// ===============================
// ======== XDG WM EVENTS ========
// ===============================

static void xdg_wm_base_ping(void* userData, struct xdg_wm_base* xdg_wm_base, uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

// ====================================
// ======== XDG SURFACE EVENTS ========
// ====================================

static void xdg_surface_configure(
    void *userData,
    struct xdg_surface* xdg_surface,
    uint32_t serial)
{
    xdg_surface_ack_configure(xdg_surface, serial);
}

static const struct xdg_surface_listener XDG_SURFACE_LISTENER = {
    .configure = xdg_surface_configure,
};

// =====================================
// ======== XDG TOPLEVEL EVENTS ========
// =====================================

static void xdg_toplevel_handle_configure(
    void*                userData,
    struct xdg_toplevel* xdg_toplevel,
    int32_t              width,
    int32_t              height,
    struct wl_array*     states)
{
	LinuxWindowWayland* window = static_cast<LinuxWindowWayland*>(userData);
    LLGL_ASSERT(width >= 0, "Width is zero");
    LLGL_ASSERT(height >= 0, "Height is zero");

    LinuxWindowWayland::State& state = window->GetState();

	if ((width == 0 || height == 0) ||
        (width == state.size.width && height == state.size.height))
    {
		return;
	}

    window->SetSizeInternal(Extent2D(width, height));
}

static void xdg_toplevel_handle_close(void* userData, struct xdg_toplevel* toplevel)
{
	LinuxWindowWayland* window = static_cast<LinuxWindowWayland*>(userData);
    window->GetState().shouldClose = true;
}

static const struct xdg_toplevel_listener XDG_TOPLEVEL_LISTENER = {
	.configure = xdg_toplevel_handle_configure,
	.close = xdg_toplevel_handle_close,
};

// =================================
// ======== REGISTRY EVENTS ========
// =================================

void RegistryHandleGlobal(
    void* userData,
    struct wl_registry* registry,
    uint32_t name,
    const char* interface,
    uint32_t version)
{
    if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        g_waylandState.compositor = static_cast<struct wl_compositor*>(
            wl_registry_bind(registry, name, &wl_compositor_interface, std::min(3u, version))
        );
    }
    else if (strcmp(interface, wl_subcompositor_interface.name) == 0)
    {
        g_waylandState.subcompositor = static_cast<struct wl_subcompositor*>(wl_registry_bind(registry, name, &wl_subcompositor_interface, 1));
    }
    else if ((strcmp(interface, wl_seat_interface.name) == 0))
    {
        g_waylandState.seat = static_cast<struct wl_seat*>(
            wl_registry_bind(registry, name, &wl_seat_interface, std::min(4u, version))
        );

        wl_seat_add_listener(g_waylandState.seat, &SEAT_LISTENER, nullptr);

        if (wl_seat_get_version(g_waylandState.seat) >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
        {
            g_waylandState.keyRepeatTimerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
        }
    }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
    {
        g_waylandState.xdg_wm_base = static_cast<struct xdg_wm_base*>(
            wl_registry_bind(registry, name, &xdg_wm_base_interface, 1)
        );
        xdg_wm_base_add_listener(g_waylandState.xdg_wm_base, &xdg_wm_base_listener, nullptr);
    }
    else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0)
    {
        g_waylandState.decorationManager = static_cast<struct zxdg_decoration_manager_v1*>(wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1));
    }
    else if (strcmp(interface, wp_viewporter_interface.name) == 0)
    {
        g_waylandState.viewporter = static_cast<struct wp_viewporter*>(wl_registry_bind(registry, name, &wp_viewporter_interface, 1));
    }
}

void RegistryHandleRemove(void* userData, struct wl_registry* registry, uint32_t name)
{
    WaylandState* state = static_cast<WaylandState*>(userData);
}

const static wl_registry_listener REGISTRY_LISTENER = {
    RegistryHandleGlobal,
    RegistryHandleRemove
};

static bool FlushDisplay()
{
    while (wl_display_flush(g_waylandState.display) == -1)
    {
        if (errno != EAGAIN)
            return false;

        struct pollfd fd = { wl_display_get_fd(g_waylandState.display), POLLOUT };

        while (poll(&fd, 1, -1) == -1)
        {
            if (errno != EINTR && errno != EAGAIN)
                return false;
        }
    }
    return true;
}

static bool PollPosix(struct pollfd* fds, nfds_t count, double* timeout)
{
    for (;;)
    {
        if (timeout)
        {
            const uint64_t base = Timer::Tick();

#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__CYGWIN__)
            const time_t seconds = (time_t) *timeout;
            const long nanoseconds = (long) ((*timeout - seconds) * 1e9);
            const struct timespec ts = { seconds, nanoseconds };
            const int result = ppoll(fds, count, &ts, NULL);
#elif defined(__NetBSD__)
            const time_t seconds = (time_t) *timeout;
            const long nanoseconds = (long) ((*timeout - seconds) * 1e9);
            const struct timespec ts = { seconds, nanoseconds };
            const int result = pollts(fds, count, &ts, NULL);
#else
            const int milliseconds = (int) (*timeout * 1e3);
            const int result = poll(fds, count, milliseconds);
#endif
            const int error = errno; // clock_gettime may overwrite our error

            *timeout -= (Timer::Tick() - base) /
                (double) Timer::Frequency();

            if (result > 0)
                return true;
            else if (result == -1 && error != EINTR && error != EAGAIN)
                return false;
            else if (*timeout <= 0.0)
                return false;
        }
        else
        {
            const int result = poll(fds, count, -1);
            if (result > 0)
                return true;
            else if (result == -1 && errno != EINTR && errno != EAGAIN)
                return false;
        }
    }
}

void HandleWaylandEvents(double* timeout)
{
    bool event = false;

    enum { DISPLAY_FD, KEYREPEAT_FD, LIBDECOR_FD };
    struct pollfd fds[3];
    fds[DISPLAY_FD] = { wl_display_get_fd(g_waylandState.display), POLLIN };
    fds[KEYREPEAT_FD] = { g_waylandState.keyRepeatTimerfd, POLLIN };
    fds[LIBDECOR_FD] = { -1, POLLIN };

    if (g_waylandState.libdecor.context)
        fds[LIBDECOR_FD].fd = libdecor_get_fd(g_waylandState.libdecor.context);

    while (!event)
    {
        while (wl_display_prepare_read(g_waylandState.display) != 0)
        {
            if (wl_display_dispatch_pending(g_waylandState.display) > 0)
                return;
        }

        // If an error other than EAGAIN happens, we have likely been disconnected
        // from the Wayland session; try to handle that the best we can.
        if (!FlushDisplay())
        {
            wl_display_cancel_read(g_waylandState.display);

            for (LinuxWindowWayland* window : LinuxWaylandContext::GetWindows()) {
                window->GetState().shouldClose = true;
            }

            return;
        }

        if (!PollPosix(fds, sizeof(fds) / sizeof(fds[0]), timeout))
        {
            wl_display_cancel_read(g_waylandState.display);
            return;
        }

        if (fds[DISPLAY_FD].revents & POLLIN)
        {
            wl_display_read_events(g_waylandState.display);
            if (wl_display_dispatch_pending(g_waylandState.display) > 0)
                event = true;
        }
        else
            wl_display_cancel_read(g_waylandState.display);

        if (fds[KEYREPEAT_FD].revents & POLLIN)
        {
            uint64_t repeats;

            if (read(g_waylandState.keyRepeatTimerfd, &repeats, sizeof(repeats)) == 8)
            {
                for (uint64_t i = 0; i < repeats; i++)
                {
                    g_waylandState.keyboardFocus->ProcessKeyEvent(translateKey(g_waylandState.keyRepeatScancode), true);
                }

                event = true;
            }
        }

        if (fds[LIBDECOR_FD].revents & POLLIN)
        {
            if (libdecor_dispatch(g_waylandState.libdecor.context, 0) > 0)
                event = true;
        }
    }
}

static void ResizeFrameBuffer(LinuxWindowWayland::State& state) {
    state.framebufferSize.width = state.size.width * state.framebufferScale;
    state.framebufferSize.height = state.size.height * state.framebufferScale;
}

static bool ResizeWindow(LinuxWindowWayland& window, int width, int height)
{
    LinuxWindowWayland::State& state = window.GetState();

    width = std::max(width, 1);
    height = std::max(height, 1);

    if (width == state.size.width && height == state.size.height)
        return false;

    window.SetSizeInternal(Extent2D(width, height));

    ResizeFrameBuffer(state);

    // if (state.scalingViewport)
    // {
    //     wp_viewport_set_destination(state.scalingViewport,
    //                                 state.size.width,
    //                                 state.size.height);
    // }

    if (state.fallback.decorations)
    {
        wp_viewport_set_destination(state.fallback.top.viewport,
                                    state.size.width,
                                    DECORATION_CAPTION_HEIGHT);
        wl_surface_commit(state.fallback.top.surface);

        wp_viewport_set_destination(state.fallback.left.viewport,
                                    DECORATION_BORDER_SIZE,
                                    state.size.height + DECORATION_CAPTION_HEIGHT);
        wl_surface_commit(state.fallback.left.surface);

        wl_subsurface_set_position(state.fallback.right.subsurface,
                                state.size.width, -DECORATION_CAPTION_HEIGHT);
        wp_viewport_set_destination(state.fallback.right.viewport,
                                    DECORATION_BORDER_SIZE,
                                    state.size.height + DECORATION_CAPTION_HEIGHT);
        wl_surface_commit(state.fallback.right.surface);

        wl_subsurface_set_position(state.fallback.bottom.subsurface,
                                -DECORATION_BORDER_SIZE, state.size.height);
        wp_viewport_set_destination(state.fallback.bottom.viewport,
                                    state.size.width + DECORATION_BORDER_SIZE * 2,
                                    DECORATION_BORDER_SIZE);
        wl_surface_commit(state.fallback.bottom.surface);
    }

    return true;
}

static void LibdecorFrameHandleConfigure(
                        struct libdecor_frame* frame,
                        struct libdecor_configuration* config,
                        void* userData)
{
    LinuxWindowWayland* window = static_cast<LinuxWindowWayland*>(userData);
    LinuxWindowWayland::State& state = window->GetState();

    int width, height;

    enum libdecor_window_state windowState;
    bool fullscreen;
    bool activated;
    bool maximized;

    if (libdecor_configuration_get_window_state(config, &windowState))
    {
        fullscreen = (windowState & LIBDECOR_WINDOW_STATE_FULLSCREEN) != 0;
        activated = (windowState & LIBDECOR_WINDOW_STATE_ACTIVE) != 0;
        maximized = (windowState & LIBDECOR_WINDOW_STATE_MAXIMIZED) != 0;
    }
    else
    {
        fullscreen = state.fullscreen;
        activated = state.activated;
        maximized = state.maximized;
    }

    if (!libdecor_configuration_get_content_size(config, frame, &width, &height))
    {
        width = state.size.width;
        height = state.size.height;
    }

    struct libdecor_state* frameState = libdecor_state_new(width, height);
    libdecor_frame_commit(frame, frameState, config);
    libdecor_state_free(frameState);

    if (state.activated != activated)
    {
        state.activated = activated;
        if (!state.activated)
        {
            // TODO
            // if (window->monitor && window->autoIconify)
            //     libdecor_frame_set_minimized(state.libdecor.frame);
        }
    }

    if (state.maximized != maximized)
    {
        state.maximized = maximized;
    }

    state.fullscreen = fullscreen;

    bool damaged = false;

    if (!state.visible)
    {
        state.visible = true;
        damaged = true;
    }

    if (ResizeWindow(*window, width, height))
    {
        damaged = true;
    }

    if (damaged)
        window->PostUpdate();
    else
        wl_surface_commit(state.wl.surface);
}

static void LibdecorFrameHandleClose(struct libdecor_frame* frame, void* userData)
{
    LinuxWindowWayland* window = static_cast<LinuxWindowWayland*>(userData);
    LinuxWindowWayland::State& state = window->GetState();
    state.shouldClose = true;
}

static void LibdecorFrameHandleCommit(struct libdecor_frame* frame, void* userData)
{
    LinuxWindowWayland* window = static_cast<LinuxWindowWayland*>(userData);
    LinuxWindowWayland::State& state = window->GetState();
    wl_surface_commit(state.wl.surface);
}

static void LibdecorFrameHandleDismissPopup(struct libdecor_frame* frame, const char* seatName, void* userData)
{
}

static struct libdecor_frame_interface libdecorFrameInterface =
{
    LibdecorFrameHandleConfigure,
    LibdecorFrameHandleClose,
    LibdecorFrameHandleCommit,
    LibdecorFrameHandleDismissPopup
};

static bool CreateLibdecorFrame(LinuxWindowWayland& window)
{
    // Allow libdecor to finish initialization of itself and its plugin
    while (!g_waylandState.libdecor.ready)
        HandleWaylandEvents(nullptr);

    LinuxWindowWayland::State& state = window.GetState();

    state.libdecor.frame = libdecor_decorate(g_waylandState.libdecor.context, state.wl.surface, &libdecorFrameInterface, &window);

    if (!state.libdecor.frame)
    {
        LLGL::Log::Errorf("Wayland: Failed to create libdecor frame");
        return false;
    }

    struct libdecor_state* frameState = libdecor_state_new(state.size.width, state.size.height);
    libdecor_frame_commit(state.libdecor.frame, frameState, NULL);
    libdecor_state_free(frameState);

    libdecor_frame_set_title(state.libdecor.frame, window.GetTitle());

    if (!state.resizable)
    {
        libdecor_frame_unset_capabilities(state.libdecor.frame, LIBDECOR_ACTION_RESIZE);
    }

    // if (window->monitor)
    // {
        // libdecor_frame_set_fullscreen(state.libdecor.frame,
        //                               window->monitor->wl.output);
        // setIdleInhibitor(window, true);
    // }
    // else
    // {
        // if (state.maximized)
        //     libdecor_frame_set_maximized(state.libdecor.frame);

        // if (!state.decorated)
        //     libdecor_frame_set_visibility(state.libdecor.frame, false);

        // setIdleInhibitor(window, false);
    // }

    libdecor_frame_map(state.libdecor.frame);
    wl_display_roundtrip(g_waylandState.display);
    return true;
}

static void InitKeyTables()
{
    memset(g_waylandState.keycodes, static_cast<int>(Key::Any), sizeof(g_waylandState.keycodes));

    g_waylandState.keycodes[KEY_1]          = Key::D1;
    g_waylandState.keycodes[KEY_2]          = Key::D2;
    g_waylandState.keycodes[KEY_3]          = Key::D3;
    g_waylandState.keycodes[KEY_4]          = Key::D4;
    g_waylandState.keycodes[KEY_5]          = Key::D5;
    g_waylandState.keycodes[KEY_6]          = Key::D6;
    g_waylandState.keycodes[KEY_7]          = Key::D7;
    g_waylandState.keycodes[KEY_8]          = Key::D8;
    g_waylandState.keycodes[KEY_9]          = Key::D9;
    g_waylandState.keycodes[KEY_0]          = Key::D0;
    g_waylandState.keycodes[KEY_SPACE]      = Key::Space;
    g_waylandState.keycodes[KEY_MINUS]      = Key::Minus;
    g_waylandState.keycodes[KEY_Q]          = Key::Q;
    g_waylandState.keycodes[KEY_W]          = Key::W;
    g_waylandState.keycodes[KEY_E]          = Key::E;
    g_waylandState.keycodes[KEY_R]          = Key::R;
    g_waylandState.keycodes[KEY_T]          = Key::T;
    g_waylandState.keycodes[KEY_Y]          = Key::Y;
    g_waylandState.keycodes[KEY_U]          = Key::U;
    g_waylandState.keycodes[KEY_I]          = Key::I;
    g_waylandState.keycodes[KEY_O]          = Key::O;
    g_waylandState.keycodes[KEY_P]          = Key::P;
    // g_waylandState.keycodes[KEY_LEFTBRACE]  = Key::LeftBracket;
    // g_waylandState.keycodes[KEY_RIGHTBRACE] = Key::RightBracket;
    g_waylandState.keycodes[KEY_A]          = Key::A;
    g_waylandState.keycodes[KEY_S]          = Key::S;
    g_waylandState.keycodes[KEY_D]          = Key::D;
    g_waylandState.keycodes[KEY_F]          = Key::F;
    g_waylandState.keycodes[KEY_G]          = Key::G;
    g_waylandState.keycodes[KEY_H]          = Key::H;
    g_waylandState.keycodes[KEY_J]          = Key::J;
    g_waylandState.keycodes[KEY_K]          = Key::K;
    g_waylandState.keycodes[KEY_L]          = Key::L;
    // g_waylandState.keycodes[KEY_SEMICOLON]  = Key::Semicolon;
    // g_waylandState.keycodes[KEY_APOSTROPHE] = Key::Apostrophe;
    g_waylandState.keycodes[KEY_Z]          = Key::Z;
    g_waylandState.keycodes[KEY_X]          = Key::X;
    g_waylandState.keycodes[KEY_C]          = Key::C;
    g_waylandState.keycodes[KEY_V]          = Key::V;
    g_waylandState.keycodes[KEY_B]          = Key::B;
    g_waylandState.keycodes[KEY_N]          = Key::N;
    g_waylandState.keycodes[KEY_M]          = Key::M;
    g_waylandState.keycodes[KEY_COMMA]      = Key::Comma;
    g_waylandState.keycodes[KEY_DOT]        = Key::Period;
    // g_waylandState.keycodes[KEY_SLASH]      = Key::Slash;
    // g_waylandState.keycodes[KEY_BACKSLASH]  = Key::Backslash;
    g_waylandState.keycodes[KEY_ESC]        = Key::Escape;
    g_waylandState.keycodes[KEY_TAB]        = Key::Tab;
    g_waylandState.keycodes[KEY_LEFTSHIFT]  = Key::LShift;
    g_waylandState.keycodes[KEY_RIGHTSHIFT] = Key::RShift;
    g_waylandState.keycodes[KEY_LEFTCTRL]   = Key::LControl;
    g_waylandState.keycodes[KEY_RIGHTCTRL]  = Key::RControl;
    // g_waylandState.keycodes[KEY_LEFTALT]    = Key::LAlt;
    // g_waylandState.keycodes[KEY_RIGHTALT]   = Key::RAlt;
    g_waylandState.keycodes[KEY_LEFTMETA]   = Key::LWin;
    g_waylandState.keycodes[KEY_RIGHTMETA]  = Key::RWin;
    g_waylandState.keycodes[KEY_NUMLOCK]    = Key::NumLock;
    // g_waylandState.keycodes[KEY_CAPSLOCK]   = Key::CapsLock;
    g_waylandState.keycodes[KEY_PRINT]      = Key::Print;
    g_waylandState.keycodes[KEY_SCROLLLOCK] = Key::ScrollLock;
    g_waylandState.keycodes[KEY_PAUSE]      = Key::Pause;
    g_waylandState.keycodes[KEY_DELETE]     = Key::Delete;
    g_waylandState.keycodes[KEY_BACKSPACE]  = Key::Back;
    g_waylandState.keycodes[KEY_ENTER]      = Key::Return;
    g_waylandState.keycodes[KEY_HOME]       = Key::Home;
    g_waylandState.keycodes[KEY_END]        = Key::End;
    g_waylandState.keycodes[KEY_PAGEUP]     = Key::PageUp;
    g_waylandState.keycodes[KEY_PAGEDOWN]   = Key::PageDown;
    g_waylandState.keycodes[KEY_INSERT]     = Key::Insert;
    g_waylandState.keycodes[KEY_LEFT]       = Key::Left;
    g_waylandState.keycodes[KEY_RIGHT]      = Key::Right;
    g_waylandState.keycodes[KEY_DOWN]       = Key::Down;
    g_waylandState.keycodes[KEY_UP]         = Key::Up;
    g_waylandState.keycodes[KEY_F1]         = Key::F1;
    g_waylandState.keycodes[KEY_F2]         = Key::F2;
    g_waylandState.keycodes[KEY_F3]         = Key::F3;
    g_waylandState.keycodes[KEY_F4]         = Key::F4;
    g_waylandState.keycodes[KEY_F5]         = Key::F5;
    g_waylandState.keycodes[KEY_F6]         = Key::F6;
    g_waylandState.keycodes[KEY_F7]         = Key::F7;
    g_waylandState.keycodes[KEY_F8]         = Key::F8;
    g_waylandState.keycodes[KEY_F9]         = Key::F9;
    g_waylandState.keycodes[KEY_F10]        = Key::F10;
    g_waylandState.keycodes[KEY_F11]        = Key::F11;
    g_waylandState.keycodes[KEY_F12]        = Key::F12;
    g_waylandState.keycodes[KEY_F13]        = Key::F13;
    g_waylandState.keycodes[KEY_F14]        = Key::F14;
    g_waylandState.keycodes[KEY_F15]        = Key::F15;
    g_waylandState.keycodes[KEY_F16]        = Key::F16;
    g_waylandState.keycodes[KEY_F17]        = Key::F17;
    g_waylandState.keycodes[KEY_F18]        = Key::F18;
    g_waylandState.keycodes[KEY_F19]        = Key::F19;
    g_waylandState.keycodes[KEY_F20]        = Key::F20;
    g_waylandState.keycodes[KEY_F21]        = Key::F21;
    g_waylandState.keycodes[KEY_F22]        = Key::F22;
    g_waylandState.keycodes[KEY_F23]        = Key::F23;
    g_waylandState.keycodes[KEY_F24]        = Key::F24;
    g_waylandState.keycodes[KEY_KPSLASH]    = Key::KeypadDivide;
    g_waylandState.keycodes[KEY_KPASTERISK] = Key::KeypadMultiply;
    g_waylandState.keycodes[KEY_KPMINUS]    = Key::KeypadMinus;
    g_waylandState.keycodes[KEY_KPPLUS]     = Key::KeypadPlus;
    g_waylandState.keycodes[KEY_KP0]        = Key::Keypad0;
    g_waylandState.keycodes[KEY_KP1]        = Key::Keypad1;
    g_waylandState.keycodes[KEY_KP2]        = Key::Keypad2;
    g_waylandState.keycodes[KEY_KP3]        = Key::Keypad3;
    g_waylandState.keycodes[KEY_KP4]        = Key::Keypad4;
    g_waylandState.keycodes[KEY_KP5]        = Key::Keypad5;
    g_waylandState.keycodes[KEY_KP6]        = Key::Keypad6;
    g_waylandState.keycodes[KEY_KP7]        = Key::Keypad7;
    g_waylandState.keycodes[KEY_KP8]        = Key::Keypad8;
    g_waylandState.keycodes[KEY_KP9]        = Key::Keypad9;
    g_waylandState.keycodes[KEY_KPDOT]      = Key::KeypadDecimal;
    // g_waylandState.keycodes[KEY_KPEQUAL]    = Key::KeypadEqual;
    // g_waylandState.keycodes[KEY_KPENTER]    = Key::KeypadEnter;
}

static void CreateFallbackEdge(LinuxWindowWayland& window,
                               FallbackEdge* edge,
                               struct wl_surface* parent,
                               struct wl_buffer* buffer,
                               int x, int y,
                               int width, int height)
{
    edge->surface = wl_compositor_create_surface(g_waylandState.compositor);
    wl_surface_set_user_data(edge->surface, &window);
    wl_proxy_set_tag((struct wl_proxy*) edge->surface, &g_waylandState.tag);
    edge->subsurface = wl_subcompositor_get_subsurface(g_waylandState.subcompositor,
                                                       edge->surface, parent);
    wl_subsurface_set_position(edge->subsurface, x, y);
    edge->viewport = wp_viewporter_get_viewport(g_waylandState.viewporter,
                                                edge->surface);
    wp_viewport_set_destination(edge->viewport, width, height);
    wl_surface_attach(edge->surface, buffer, 0, 0);

    struct wl_region* region = wl_compositor_create_region(g_waylandState.compositor);
    wl_region_add(region, 0, 0, width, height);
    wl_surface_set_opaque_region(edge->surface, region);
    wl_surface_commit(edge->surface);
    wl_region_destroy(region);
}

static void RandName(char *buf)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	long r = ts.tv_nsec;
	for (int i = 0; i < 6; ++i) {
		buf[i] = 'A'+(r&15)+(r&16)*2;
		r >>= 5;
	}
}

static int CreateShmFile()
{
	int retries = 100;
	do {
		char name[] = "/wl_shm-XXXXXX";
		RandName(name + sizeof(name) - 7);
		--retries;
		int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
		if (fd >= 0) {
			shm_unlink(name);
			return fd;
		}
	} while (retries > 0 && errno == EEXIST);
	return -1;
}

int AllocateShmFile(size_t size)
{
	int fd = CreateShmFile();
	if (fd < 0)
		return -1;
	int ret;
	do {
		ret = ftruncate(fd, size);
	} while (ret < 0 && errno == EINTR);
	if (ret < 0) {
		close(fd);
		return -1;
	}
	return fd;
}

static struct wl_buffer* CreateShmBuffer(const Image& image)
{
    const int stride = image.width * 4;
    const int length = image.width * image.height * 4;

    const int fd = AllocateShmFile(length);
    if (fd < 0)
    {
        LLGL::Log::Errorf("Wayland: Failed to create buffer file of size %d: %s", length, strerror(errno));
        return NULL;
    }

    void* data = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
    {
        LLGL::Log::Errorf("Wayland: Failed to map file: %s", strerror(errno));
        close(fd);
        return NULL;
    }

    struct wl_shm_pool* pool = wl_shm_create_pool(g_waylandState.shm, fd, length);

    close(fd);

    uint8_t* source = image.pixels;
    uint8_t* target = static_cast<uint8_t*>(data);
    for (int i = 0;  i < image.width * image.height;  i++, source += 4)
    {
        uint32_t alpha = source[3];

        *target++ = (uint8_t) ((source[2] * alpha) / 255);
        *target++ = (uint8_t) ((source[1] * alpha) / 255);
        *target++ = (uint8_t) ((source[0] * alpha) / 255);
        *target++ = (uint8_t) alpha;
    }

    struct wl_buffer* buffer =
        wl_shm_pool_create_buffer(pool, 0,
                                  image.width,
                                  image.height,
                                  stride, WL_SHM_FORMAT_ARGB8888);
    munmap(data, length);
    wl_shm_pool_destroy(pool);

    return buffer;
}

static void CreateFallbackDecorations(LinuxWindowWayland& window)
{
    LinuxWindowWayland::State& state = window.GetState();

    unsigned char data[] = { 224, 224, 224, 255 };
    const Image image = { 1, 1, data };

    if (!g_waylandState.viewporter)
        return;

    if (!state.fallback.buffer)
        state.fallback.buffer = CreateShmBuffer(image);
    if (!state.fallback.buffer)
        return;

    CreateFallbackEdge(window, &state.fallback.top, state.wl.surface,
                       state.fallback.buffer,
                       0, -DECORATION_CAPTION_HEIGHT,
                       state.size.width, DECORATION_CAPTION_HEIGHT);
    CreateFallbackEdge(window, &state.fallback.left, state.wl.surface,
                       state.fallback.buffer,
                       -DECORATION_BORDER_SIZE, -DECORATION_CAPTION_HEIGHT,
                       DECORATION_BORDER_SIZE, state.size.height + DECORATION_CAPTION_HEIGHT);
    CreateFallbackEdge(window, &state.fallback.right, state.wl.surface,
                       state.fallback.buffer,
                       state.size.width, -DECORATION_CAPTION_HEIGHT,
                       DECORATION_BORDER_SIZE, state.size.height + DECORATION_CAPTION_HEIGHT);
    CreateFallbackEdge(window, &state.fallback.bottom, state.wl.surface,
                       state.fallback.buffer,
                       -DECORATION_BORDER_SIZE, state.size.height,
                       state.size.width + DECORATION_BORDER_SIZE * 2, DECORATION_BORDER_SIZE);

    state.fallback.decorations = true;
}

static void DestroyFallbackEdge(FallbackEdge& edge)
{
    if (edge.subsurface)
        wl_subsurface_destroy(edge.subsurface);
    if (edge.surface)
        wl_surface_destroy(edge.surface);
    if (edge.viewport)
        wp_viewport_destroy(edge.viewport);

    edge.surface = NULL;
    edge.subsurface = NULL;
    edge.viewport = NULL;
}

static void DestroyFallbackDecorations(LinuxWindowWayland::State& state)
{
    state.fallback.decorations = false;

    DestroyFallbackEdge(state.fallback.top);
    DestroyFallbackEdge(state.fallback.left);
    DestroyFallbackEdge(state.fallback.right);
    DestroyFallbackEdge(state.fallback.bottom);
}

static void XdgDecorationHandleConfigure(void* userData,
                                         struct zxdg_toplevel_decoration_v1* decoration,
                                         uint32_t mode)
{
    LinuxWindowWayland* window = static_cast<LinuxWindowWayland*>(userData);
    LinuxWindowWayland::State& state = window->GetState();

    state.xdg.decorationMode = mode;

    if (mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE)
    {
        // TODO
        // if (!window->monitor)
        //     CreateFallbackDecorations(*window);
    }
    else
        DestroyFallbackDecorations(state);
}

static const struct zxdg_toplevel_decoration_v1_listener XDG_DECORATION_LISTENER =
{
    XdgDecorationHandleConfigure,
};

static bool CreateXdgShellObjects(LinuxWindowWayland& window)
{
    LinuxWindowWayland::State& state = window.GetState();

    state.xdg.surface = xdg_wm_base_get_xdg_surface(g_waylandState.xdg_wm_base, state.wl.surface);
    if (!state.xdg.surface)
    {
        LLGL::Log::Errorf("Wayland: Failed to create xdg-surface for window");
        return false;
    }

    xdg_surface_add_listener(state.xdg.surface, &XDG_SURFACE_LISTENER, &window);

    state.xdg.toplevel = xdg_surface_get_toplevel(state.xdg.surface);
    if (!state.xdg.toplevel)
    {
        LLGL::Log::Errorf("Wayland: Failed to create xdg-toplevel for window");
        return false;
    }

    xdg_toplevel_add_listener(state.xdg.toplevel, &XDG_TOPLEVEL_LISTENER, &window);

    xdg_toplevel_set_title(state.xdg.toplevel, window.GetTitle());

    // TODO
    // if (window->monitor)
    // {
    //     xdg_toplevel_set_fullscreen(state.xdg.toplevel, window->monitor->wl.output);
    //     setIdleInhibitor(window, true);
    // }
    // else
    // {
    //     if (state.maximized)
    //         xdg_toplevel_set_maximized(state.xdg.toplevel);

    //     setIdleInhibitor(window, false);
    // }

    if (g_waylandState.decorationManager)
    {
        state.xdg.decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(g_waylandState.decorationManager, state.xdg.toplevel);
        zxdg_toplevel_decoration_v1_add_listener(state.xdg.decoration, &XDG_DECORATION_LISTENER, &window);


        // TODO: Let the user choose the mode?
        uint32_t mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
        zxdg_toplevel_decoration_v1_set_mode(state.xdg.decoration, mode);
    }
    else
    {
        // TODO
        // if (!window->monitor)
        //     CreateFallbackDecorations(window);
    }

    // UpdateXdgSizeLimits(window);

    wl_surface_commit(state.wl.surface);
    wl_display_roundtrip(g_waylandState.display);
    return true;
}

static bool CreateShellObjects(LinuxWindowWayland& window)
{
    if (g_waylandState.libdecor.context) {
        if (CreateLibdecorFrame(window))
            return true;
    }

    return CreateXdgShellObjects(window);
}

static void DestroyShellObjects(LinuxWindowWayland& window)
{
    LinuxWindowWayland::State& state = window.GetState();

    DestroyFallbackDecorations(state);

    if (state.libdecor.frame)
        libdecor_frame_unref(state.libdecor.frame);

    if (state.xdg.decoration)
        zxdg_toplevel_decoration_v1_destroy(state.xdg.decoration);

    if (state.xdg.toplevel)
        xdg_toplevel_destroy(state.xdg.toplevel);

    if (state.xdg.surface)
        xdg_surface_destroy(state.xdg.surface);

    state.libdecor.frame = NULL;
    state.xdg.decoration = NULL;
    state.xdg.decorationMode = 0;
    state.xdg.toplevel = NULL;
    state.xdg.surface = NULL;
}

void LibdecorHandleError(struct libdecor* context,
                         enum libdecor_error error,
                         const char* message)
{
    LLGL::Log::Errorf("Wayland: libdecor error %u: %s", error, message);
}

static struct libdecor_interface LIBDECOR_INTERFACE =
{
    LibdecorHandleError
};

static void LibdecorReadyCallback(void* userData,
                                  struct wl_callback* callback,
                                  uint32_t time)
{
    g_waylandState.libdecor.ready = true;
    LLGL_ASSERT(g_waylandState.libdecor.callback == callback);
    wl_callback_destroy(g_waylandState.libdecor.callback);
    g_waylandState.libdecor.callback = NULL;
}

static const struct wl_callback_listener LIBDECOR_READY_LISTENER =
{
    LibdecorReadyCallback
};

static void SetContentAreaOpaque(LinuxWindowWayland::State& state)
{
    struct wl_region* region;

    region = wl_compositor_create_region(g_waylandState.compositor);
    if (!region)
        return;

    wl_region_add(region, 0, 0, state.size.width, state.size.height);
    wl_surface_set_opaque_region(state.wl.surface, region);
    wl_region_destroy(region);
}

static void SetWindowDecorated(LinuxWindowWayland& window, bool decorated) {
    LinuxWindowWayland::State& state = window.GetState();

    if (state.libdecor.frame)
    {
        libdecor_frame_set_visibility(state.libdecor.frame, decorated);
    }
    else if (state.xdg.decoration)
    {
        uint32_t mode = 0;

        if (decorated)
            mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
        else
            mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;

        zxdg_toplevel_decoration_v1_set_mode(state.xdg.decoration, mode);
    }
    else if (state.xdg.toplevel)
    {
        if (decorated)
            CreateFallbackDecorations(window);
        else
            DestroyFallbackDecorations(state);
    }
}

/*
 * LinuxWindowWayland class
 */

LinuxWindowWayland::LinuxWindowWayland(const WindowDescriptor& desc)
{
    LinuxWaylandContext::Add(this);
    SetDesc(desc);
    Open();
}

void LinuxWindowWayland::Open()
{
    // TODO: This should not be here
    if (!g_waylandState.initialized)
    {
        InitKeyTables();

        g_waylandState.xkb.context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
        if (!g_waylandState.xkb.context)
        {
            LLGL_TRAP("Failed to initialize xkb context");
        }

        g_waylandState.display = wl_display_connect(nullptr);
        if (!g_waylandState.display)
            LLGL_TRAP("Failed to connect to the Wayland display");

        struct wl_registry* registry = wl_display_get_registry(g_waylandState.display);

        wl_registry_add_listener(registry, &REGISTRY_LISTENER, this);

        wl_display_roundtrip(g_waylandState.display);
        wl_display_roundtrip(g_waylandState.display);

        g_waylandState.libdecor.context = libdecor_new(g_waylandState.display, &LIBDECOR_INTERFACE);
        if (g_waylandState.libdecor.context)
        {
            // Perform an initial dispatch and flush to get the init started
            libdecor_dispatch(g_waylandState.libdecor.context, 0);

            // Create sync point to "know" when libdecor is ready for use
            g_waylandState.libdecor.callback = wl_display_sync(g_waylandState.display);
            wl_callback_add_listener(g_waylandState.libdecor.callback, &LIBDECOR_READY_LISTENER, nullptr);
        }

        if (!g_waylandState.compositor)
            LLGL_TRAP("Failed to get Wayland compositor");

        g_waylandState.tag = "LLGL";

        g_waylandState.initialized = true;
    }

    state_.wl.surface = wl_compositor_create_surface(g_waylandState.compositor);
    if (!state_.wl.surface)
        LLGL_TRAP("Failed to get Wayland surface");

    wl_proxy_set_tag(reinterpret_cast<struct wl_proxy*>(state_.wl.surface), &g_waylandState.tag);
    wl_surface_add_listener(state_.wl.surface, &SURFACE_LISTENER, nullptr);
    wl_surface_set_user_data(state_.wl.surface, this);

    wl_surface_commit(state_.wl.surface);
    wl_display_roundtrip(g_waylandState.display);

    ResizeWindow(*this, desc_.size.width, desc_.size.height);

    SetContentAreaOpaque(state_);

    if (state_.visible)
    {
        CreateShellObjects(*this);
    }
}

void LinuxWindowWayland::ProcessEvents()
{
    if (state_.shouldClose)
        PostQuit();
}

void LinuxWindowWayland::ProcessMotionEvent(int xpos, int ypos)
{
    const Offset2D mousePos { xpos, ypos };
    PostLocalMotion(mousePos);
    PostGlobalMotion({ mousePos.x - state_.prevMousePos.x, mousePos.y - state_.prevMousePos.y });
    state_.prevMousePos = mousePos;
}

bool LinuxWindowWayland::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(NativeHandle))
    {
        auto* handle = static_cast<NativeHandle*>(nativeHandle);
        handle->type = NativeType::Wayland;
        handle->wayland.display = g_waylandState.display;
        handle->wayland.window  = state_.wl.surface;
        return true;
    }
    return false;
}

Offset2D LinuxWindowWayland::GetPosition() const
{
    return state_.position;
}

void LinuxWindowWayland::ProcessKeyEvent(Key key, bool pressed)
{
    if (pressed)
        PostKeyDown(key);
    else
        PostKeyUp(key);
}

void LinuxWindowWayland::ProcessMouseKeyEvent(uint32_t key, bool pressed)
{
    switch (key)
    {
        case BTN_LEFT:
            ProcessKeyEvent(Key::LButton, pressed);
            break;
        case BTN_MIDDLE:
            ProcessKeyEvent(Key::MButton, pressed);
            break;
        case BTN_RIGHT:
            ProcessKeyEvent(Key::RButton, pressed);
            break;
    }
}

void LinuxWindowWayland::ProcessWheelMotionEvent(int motion)
{
    PostWheelMotion(motion);
}

void LinuxWindowWayland::ProcessFocusEvent(bool focused)
{
    if (focused)
        PostGetFocus();
    else
        PostLostFocus();
}

Extent2D LinuxWindowWayland::GetSize(bool useClientArea) const
{
    // TODO: utilize useClientArea parameter
    return state_.size;
}

void LinuxWindowWayland::Show(bool show)
{
    bool visible = show;

    if (visible != state_.visible)
    {
        if (visible && (!state_.libdecor.frame && !state_.xdg.toplevel))
        {
            visible = CreateShellObjects(*this);
        }
        else
        {
            DestroyShellObjects(*this);
            wl_surface_attach(state_.wl.surface, NULL, 0, 0);
            wl_surface_commit(state_.wl.surface);
        }
    }

    state_.visible = visible;
}

void LinuxWindowWayland::SetPosition(const LLGL::Offset2D& position)
{
    // A Wayland client can't set its position,
    // so should we leave this function empty or log a warning?
}

bool LinuxWindowWayland::IsShown() const
{
    return state_.visible;
}

Extent2D LinuxWindowWayland::GetContentSize() const
{
    return state_.framebufferSize;
}

void LinuxWindowWayland::SetSize(const LLGL::Extent2D& size, bool useClientArea)
{
    // TODO: utilize useClientArea parameter
    if (!ResizeWindow(*this, size.width, size.height))
        return;

    if (state_.libdecor.frame)
    {
        struct libdecor_state* frameState = libdecor_state_new(size.width, size.height);
        libdecor_frame_commit(state_.libdecor.frame, frameState, NULL);
        libdecor_state_free(frameState);
    }
}

WindowDescriptor LinuxWindowWayland::GetDesc() const
{
    return desc_;
}

UTF8String LinuxWindowWayland::GetTitle() const
{
    return desc_.title;
}

void LinuxWindowWayland::SetDesc(const LLGL::WindowDescriptor& desc)
{
    desc_ = desc;
    state_.size = desc.size;
    state_.framebufferSize = desc.size;
    state_.visible = ((desc_.flags & WindowFlags::Visible) != 0);
    state_.resizable = ((desc_.flags & WindowFlags::Resizable) != 0);
    state_.decorated = ((desc_.flags & WindowFlags::Borderless) == 0);
    SetWindowDecorated(*this, state_.decorated);
}

void LinuxWindowWayland::SetTitle(const LLGL::UTF8String& title)
{
    desc_.title = title;

    if (state_.libdecor.frame)
        libdecor_frame_set_title(state_.libdecor.frame, title);
    else if (state_.xdg.toplevel)
        xdg_toplevel_set_title(state_.xdg.toplevel, title);
}

void LinuxWindowWayland::SetSizeInternal(Extent2D size)
{
    state_.size = size;
    PostResize(size);
}

LinuxWindowWayland::State& LinuxWindowWayland::GetState()
{
    return state_;
}

LinuxWindowWayland::~LinuxWindowWayland()
{
    LinuxWaylandContext::Remove(this);

    DestroyShellObjects(*this);

    if (state_.fallback.buffer)
        wl_buffer_destroy(state_.fallback.buffer);

    if (state_.wl.surface)
        wl_surface_destroy(state_.wl.surface);
}

} // /namespace LLGL

#endif


// ================================================================================
