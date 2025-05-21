/*
 * LinuxWindowWayland.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#if LLGL_LINUX_ENABLE_WAYLAND

#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Display.h>
#include "LinuxWindowWayland.h"
#include "../../Core/Exception.h"
#include "../../Core/Assertion.h"

#include <unistd.h>
#include <sys/mman.h>
#include <wayland-client.h>
#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>

struct wayland_state {
    struct wl_display* display;
    struct wl_registry* registry;
    struct wl_compositor* compositor;
    struct wl_seat* seat;

    struct wl_pointer* pointer;
    LLGL::LinuxWindowWayland* pointerFocus;
    uint32_t serial;
    uint32_t pointerEnterSerial;

    struct wl_keyboard* keyboard;
    LLGL::LinuxWindowWayland* keyboardFocus;

    struct xdg_wm_base *xdg_wm_base;

    const char* tag;

    int32_t keyRepeatRate;
    int32_t keyRepeatDelay;
    int keyRepeatScancode;

    LLGL::Key keycodes[256];

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
};

static wayland_state g_waylandState;

namespace LLGL
{

// ================================
// ======== SURFACE EVENTS ========
// ================================

void SurfaceHandleEnter(void* userData, struct wl_surface* surface, struct wl_output* output)
{
    
}

void SurfaceHandleLeave(void* userData, struct wl_surface* surface, struct wl_output* output)
{
    
}

const static wl_surface_listener surfaceListener = {
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

    if (surface == state.wl_surface)
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

const static wl_pointer_listener pointerListener = {
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

    mapStr = static_cast<char*>(mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0));
    if (mapStr == MAP_FAILED) {
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

    if (surface != window->GetState().wl_surface)
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
        wl_pointer_add_listener(g_waylandState.pointer, &pointerListener, nullptr);
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

const static struct wl_seat_listener seatListener = {
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

static const struct xdg_surface_listener xdg_surface_listener = {
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
    window->GetState().closed = true;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
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
    else if ((strcmp(interface, wl_seat_interface.name) == 0))
    {
        g_waylandState.seat = static_cast<struct wl_seat*>(
            wl_registry_bind(registry, name, &wl_seat_interface, std::min(4u, version))
        );

        wl_seat_add_listener(g_waylandState.seat, &seatListener, nullptr);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
    {
        g_waylandState.xdg_wm_base = static_cast<struct xdg_wm_base*>(
            wl_registry_bind(registry, name, &xdg_wm_base_interface, 1)
        );
        xdg_wm_base_add_listener(g_waylandState.xdg_wm_base, &xdg_wm_base_listener, nullptr);
    }
}

void RegistryHandleRemove(void* userData, struct wl_registry* registry, uint32_t name)
{
    wayland_state* state = static_cast<wayland_state*>(userData);
}

const static wl_registry_listener registryListener = {
    RegistryHandleGlobal,
    RegistryHandleRemove
};

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

/*
 * LinuxWindowWayland class
 */

LinuxWindowWayland::LinuxWindowWayland(
    const WindowDescriptor& desc)
{
    state_.title = desc.title;
    state_.size = desc.size;
    state_.prevSize = desc.size;

    Open();
}

void LinuxWindowWayland::Open()
{
    // TODO: This chunk of code must be ran only once
    //////////////////////
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

    wl_registry_add_listener(registry, &registryListener, this);

    wl_display_roundtrip(g_waylandState.display);

    if (!g_waylandState.compositor)
        LLGL_TRAP("Failed to get Wayland compositor");

    g_waylandState.tag = "LLGL";

    /////////////////////

    state_.wl_surface = wl_compositor_create_surface(g_waylandState.compositor);
    if (!state_.wl_surface)
        LLGL_TRAP("Failed to get Wayland surface");

    wl_proxy_set_tag(reinterpret_cast<struct wl_proxy*>(state_.wl_surface), &g_waylandState.tag);
    wl_surface_add_listener(state_.wl_surface, &surfaceListener, nullptr);
    wl_surface_set_user_data(state_.wl_surface, this);

    state_.xdg_surface = xdg_wm_base_get_xdg_surface(g_waylandState.xdg_wm_base, state_.wl_surface);
    if (!state_.xdg_surface)
        LLGL_TRAP("Failed to get XDG surface");

    xdg_surface_add_listener(state_.xdg_surface, &xdg_surface_listener, this);

    state_.xdg_toplevel = xdg_surface_get_toplevel(state_.xdg_surface);
    if (!state_.xdg_toplevel)
        LLGL_TRAP("Failed to get XDG toplevel");

    xdg_toplevel_add_listener(state_.xdg_toplevel, &xdg_toplevel_listener, this);

    xdg_toplevel_set_title(state_.xdg_toplevel, state_.title);
    wl_surface_commit(state_.wl_surface);
}

bool LinuxWindowWayland::ProcessEvents()
{
    return !state_.closed && (wl_display_dispatch(g_waylandState.display) != -1);
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
        handle->wayland.window  = state_.wl_surface;
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
    // TODO
}

void LinuxWindowWayland::SetPosition(const LLGL::Offset2D& position)
{
    // TODO
}

bool LinuxWindowWayland::IsShown() const
{
    // TODO
    return true;
}

Extent2D LinuxWindowWayland::GetContentSize() const
{
    // TODO: Return framebuffer size
    return GetSize();
}

void LinuxWindowWayland::SetSize(const LLGL::Extent2D& size, bool useClientArea)
{
    // TODO: utilize useClientArea parameter
    xdg_surface_set_window_geometry(state_.xdg_surface, state_.position.x, state_.position.y, size.width, size.height);
}

WindowDescriptor LinuxWindowWayland::GetDesc() const
{
    return desc_;
}

UTF8String LinuxWindowWayland::GetTitle() const
{
    return state_.title;
}

void LinuxWindowWayland::SetDesc(const LLGL::WindowDescriptor&)
{
    // TODO
}

void LinuxWindowWayland::SetTitle(const LLGL::UTF8String& title)
{
    state_.title = title;
    xdg_toplevel_set_title(state_.xdg_toplevel, title);
}

void LinuxWindowWayland::SetSizeInternal(Extent2D size)
{
    state_.size = size;
    PostResize(size);
    state_.prevSize = size;
}

LinuxWindowWayland::State& LinuxWindowWayland::GetState()
{
    return state_;
}

LinuxWindowWayland::~LinuxWindowWayland()
{
    if (state_.wl_surface)
        wl_surface_destroy(state_.wl_surface);
    if (state_.xdg_surface)
        xdg_surface_destroy(state_.xdg_surface);
    if (state_.xdg_toplevel)
        xdg_toplevel_destroy(state_.xdg_toplevel);
}

} // /namespace LLGL

#endif


// ================================================================================
