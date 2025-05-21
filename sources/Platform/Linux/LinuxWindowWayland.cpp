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

#include <wayland-client.h>
#include <linux/input-event-codes.h>

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
    // TODO
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
    // TODO: Set window unfocused
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

    // TODO
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

    // TODO
}

static void KeyboardHandleRepeatInfo(void* userData,
                                     struct wl_keyboard* keyboard,
                                     int32_t rate,
                                     int32_t delay)
{
    if (keyboard != g_waylandState.keyboard)
        return;

    // TODO
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
    g_waylandState.display = wl_display_connect(nullptr);
    if (!g_waylandState.display)
        LLGL_TRAP("Failed to connect to the Wayland display");

    struct wl_registry* registry = wl_display_get_registry(g_waylandState.display);

    wl_registry_add_listener(registry, &registryListener, this);

    wl_display_roundtrip(g_waylandState.display);

    if (!g_waylandState.compositor)
        LLGL_TRAP("Failed to get Wayland compositor");

    g_waylandState.tag = "LLGL";

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

void LinuxWindowWayland::ProcessMouseKeyEvent(uint32_t key, bool pressed)
{
    switch (key)
    {
        case BTN_LEFT:
            PostMouseKeyEvent(Key::LButton, pressed);
            break;
        case BTN_MIDDLE:
            PostMouseKeyEvent(Key::MButton, pressed);
            break;
        case BTN_RIGHT:
            PostMouseKeyEvent(Key::RButton, pressed);
            break;
    }
}

void LinuxWindowWayland::PostMouseKeyEvent(Key key, bool down)
{
    if (down)
        PostKeyDown(key);
    else
        PostKeyUp(key);
}

void LinuxWindowWayland::ProcessWheelMotionEvent(int motion)
{
    PostWheelMotion(motion);
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
