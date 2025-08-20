/*
 * LinuxWaylandState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#if LLGL_LINUX_ENABLE_WAYLAND

#include <cstring>
#include <linux/input-event-codes.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
#include <sys/poll.h>

#include <LLGL/Log.h>
#include <LLGL/DisplayFlags.h>
#include <LLGL/Timer.h>

#include "../../Core/Exception.h"
#include "../../Core/Assertion.h"

#include "LinuxDisplayWayland.h"
#include "protocols/xdg-shell-client-protocol.h"

#if LLGL_WINDOWING_ENABLED
#   include "protocols/viewporter-client-protocol.h"
#   include "protocols/xdg-decoration-client-protocol.h"
#endif

#include "LinuxWaylandState.h"

namespace LLGL
{

LinuxWaylandState* LinuxWaylandState::instance_ = nullptr;

wl_registry_listener LinuxWaylandState::registryListener_ = {
    HandleRegistryGlobal,
    HandleRegistryRemove
};

#if LLGL_WINDOWING_ENABLED

wl_seat_listener LinuxWaylandState::seatListener_ = {
    HandleSeatCapabilities,
    HandleSeatName
};

wl_pointer_listener LinuxWaylandState::pointerListener_ {
    HandlePointerEnter,
    HandlePointerLeave,
    HandlePointerMotion,
    HandlePointerButton,
    HandlePointerAxis
};

wl_keyboard_listener LinuxWaylandState::keyboardListener_ = {
    HandleKeyboardKeymap,
    HandleKeyboardEnter,
    HandleKeyboardLeave,
    HandleKeyboardKey,
    HandleKeyboardModifiers,
    HandleKeyboardRepeatInfo
};

wl_callback_listener LinuxWaylandState::libdecorReadyListener_ =
{
    HandleLibdecorReady
};

xdg_wm_base_listener LinuxWaylandState::xdgWmBaseListener_ = {
    HandleXdgWmBasePing
};

#endif

wl_output_listener LinuxWaylandState::outputListener_ = {
    HandleOutputGeometry,
    HandleOutputMode,
    HandleOutputDone,
    HandleOutputScale,
    HandleOutputName,
    HandleOutputDescription
};


void LinuxWaylandState::HandleRegistryGlobal(
    void* userData,
    wl_registry* registry,
    uint32_t name,
    const char* interface,
    uint32_t version)
{
    if (strcmp(interface, wl_output_interface.name) == 0)
    {
        if (version < 2) {
            LLGL::Log::Errorf("Wayland: Unsupported output interface version");
            return;
        }

        version = std::min(version, static_cast<uint32_t>(WL_OUTPUT_NAME_SINCE_VERSION));

        wl_output* output = static_cast<wl_output*>(wl_registry_bind(registry, name, &wl_output_interface, version));

        if (output)
            GetInstance().AddWaylandOutput(output, name, version);
    }
    #if LLGL_WINDOWING_ENABLED
    else if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        GetInstance().compositor_ = static_cast<wl_compositor*>(
            wl_registry_bind(registry, name, &wl_compositor_interface, std::min(3u, version))
        );
    }
    else if (strcmp(interface, wl_subcompositor_interface.name) == 0)
    {
        GetInstance().subcompositor_ = static_cast<wl_subcompositor*>(wl_registry_bind(registry, name, &wl_subcompositor_interface, 1));
    }
    else if ((strcmp(interface, wl_seat_interface.name) == 0))
    {
        GetInstance().seat_ = static_cast<wl_seat*>(
            wl_registry_bind(registry, name, &wl_seat_interface, std::min(4u, version))
        );

        wl_seat_add_listener(GetInstance().seat_, &seatListener_, nullptr);

        if (wl_seat_get_version(GetInstance().seat_) >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
        {
            GetInstance().keyRepeatTimerfd_ = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
        }
    }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
    {
        GetInstance().xdgWmBase_ = static_cast<xdg_wm_base*>(
            wl_registry_bind(registry, name, &xdg_wm_base_interface, 1)
        );
        xdg_wm_base_add_listener(GetInstance().xdgWmBase_, &xdgWmBaseListener_, nullptr);
    }
    else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0)
    {
        GetInstance().decorationManager_ = static_cast<zxdg_decoration_manager_v1*>(wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1));
    }
    else if (strcmp(interface, wp_viewporter_interface.name) == 0)
    {
        GetInstance().viewporter_ = static_cast<wp_viewporter*>(wl_registry_bind(registry, name, &wp_viewporter_interface, 1));
    }
    #endif
}

void LinuxWaylandState::HandleRegistryRemove(void* userData, wl_registry* registry, uint32_t name)
{
}

#if LLGL_WINDOWING_ENABLED

void LinuxWaylandState::HandleXdgWmBasePing(void* userData, xdg_wm_base* xdg_wm_base, uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

void LinuxWaylandState::HandleSeatCapabilities(void* userData, wl_seat* seat, uint32_t caps)
{
    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !GetInstance().pointer_)
    {
        GetInstance().pointer_ = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(GetInstance().pointer_, &pointerListener_, nullptr);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && GetInstance().pointer_)
    {
        wl_pointer_destroy(GetInstance().pointer_);
        GetInstance().pointer_ = nullptr;
    }

    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !GetInstance().keyboard_)
    {
        GetInstance().keyboard_ = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(GetInstance().keyboard_, &keyboardListener_, nullptr);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && GetInstance().keyboard_)
    {
        wl_keyboard_destroy(GetInstance().keyboard_);
        GetInstance().keyboard_ = nullptr;
    }
}

void LinuxWaylandState::HandleSeatName(void* userData, wl_seat* seat, const char* name)
{
}

void LinuxWaylandState::HandleLibdecorReady(void* userData, wl_callback* callback, uint32_t time)
{
    GetInstance().libdecor_.ready = true;
    LLGL_ASSERT(GetInstance().libdecor_.callback == callback);
    wl_callback_destroy(GetInstance().libdecor_.callback);
    GetInstance().libdecor_.callback = NULL;
}

void LibdecorHandleError(libdecor* context,
                         enum libdecor_error error,
                         const char* message)
{
    LLGL::Log::Errorf("Wayland: libdecor error %u: %s", error, message);
}

static libdecor_interface LIBDECOR_INTERFACE =
{
    LibdecorHandleError
};

// ================================
// ======== POINTER EVENTS ========
// ================================

void LinuxWaylandState::HandlePointerEnter(
    void*               userData,
    wl_pointer*  pointer,
    uint32_t            serial,
    wl_surface*  surface,
    wl_fixed_t          surface_x,
    wl_fixed_t          surface_y)
{
    if (!surface)
        return;

    if (wl_proxy_get_tag(reinterpret_cast<wl_proxy*>(surface)) != GetInstance().GetTag())
        return;

    LinuxWindowWayland* window = static_cast<LinuxWindowWayland*>(wl_surface_get_user_data(surface));
    if (!window)
        return;

    GetInstance().serial_ = serial;
    GetInstance().pointerEnterSerial_ = serial;
    GetInstance().pointerFocus_ = window;

    LinuxWindowWayland::State& state = window->GetState();

    if (surface == state.wl.surface)
    {
        state.hovered = true;
    }
}

void LinuxWaylandState::HandlePointerLeave(
    void*              userData,
    wl_pointer* pointer,
    uint32_t           serial,
    wl_surface* surface)
{
    if (!surface)
        return;

    LinuxWindowWayland* window = GetInstance().pointerFocus_;
    if (!window)
        return;

    if (wl_proxy_get_tag(reinterpret_cast<wl_proxy*>(surface)) != GetInstance().GetTag())
        return;

    GetInstance().serial_ = serial;
    GetInstance().pointerFocus_ = nullptr;

    LinuxWindowWayland::State& state = window->GetState();

    if (state.hovered)
    {
        state.hovered = false;
    }
}

void LinuxWaylandState::HandlePointerMotion(
    void*               userData,
    wl_pointer*  pointer,
    uint32_t            time,
    wl_fixed_t          surface_x,
    wl_fixed_t          surface_y)
{
    LinuxWindowWayland* window = GetInstance().pointerFocus_;
    if (!window)
        return;

    LinuxWindowWayland::State& state = window->GetState();

    if (!state.hovered)
        return;

    const int xpos = wl_fixed_to_int(surface_x);
    const int ypos = wl_fixed_to_int(surface_y);

    window->ProcessMotionEvent(xpos, ypos);
}

void LinuxWaylandState::HandlePointerButton(
    void*               userData,
    wl_pointer*  pointer,
    uint32_t            serial,
    uint32_t            time,
    uint32_t            button,
    uint32_t            state)
{
    LinuxWindowWayland* window = GetInstance().pointerFocus_;
    if (!window)
        return;

    LinuxWindowWayland::State& windowState = window->GetState();

    if (!windowState.hovered)
        return;

    GetInstance().serial_ = serial;

    window->ProcessMouseKeyEvent(button, state == WL_POINTER_BUTTON_STATE_PRESSED);
}

void LinuxWaylandState::HandlePointerAxis(
    void*               userData,
    wl_pointer*  pointer,
    uint32_t            time,
    uint32_t            axis,
    wl_fixed_t          value)
{
    LinuxWindowWayland* window = GetInstance().pointerFocus_;
    if (!window)
        return;

    // TODO: Handle horizontal scroll?
    if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
    {
        int motion = static_cast<int>(-wl_fixed_to_double(value) / 10.0);
        window->ProcessWheelMotionEvent(motion);
    }
}

void LinuxWaylandState::HandleKeyboardEnter(
    void*               userData,
    wl_keyboard* keyboard,
    uint32_t            serial,
    wl_surface*  surface,
    wl_array*    keys)
{
    // Happens in the case we just destroyed the surface.
    if (!surface)
        return;

    if (wl_proxy_get_tag((wl_proxy*) surface) != GetTag())
        return;

    LinuxWindowWayland* window = static_cast<LinuxWindowWayland*>(wl_surface_get_user_data(surface));
    if (!window)
        return;

    if (surface != window->GetState().wl.surface)
        return;

    GetInstance().serial_ = serial;
    GetInstance().keyboardFocus_ = window;

    window->ProcessFocusEvent(true);
}

void LinuxWaylandState::HandleKeyboardLeave(
    void* userData,
    wl_keyboard* keyboard,
    uint32_t serial,
    wl_surface* surface)
{
    LinuxWindowWayland* window = static_cast<LinuxWindowWayland*>(userData);

    if (!window)
        return;

    GetInstance().serial_ = serial;
    GetInstance().keyboardFocus_ = nullptr;

    window->ProcessFocusEvent(false);
}

Key LinuxWaylandState::TranslateKey(uint32_t scancode)
{
    if (scancode < GetInstance().GetKeycodes().size())
        return GetInstance().GetKeycodes()[scancode];

    return Key::Any;
}

void LinuxWaylandState::HandleKeyboardKey(
    void* userData,
    wl_keyboard* keyboard,
    uint32_t serial,
    uint32_t time,
    uint32_t scancode,
    uint32_t state)
{
    LinuxWindowWayland* window = GetInstance().keyboardFocus_;
    if (!window)
        return;

    GetInstance().serial_ = serial;

    itimerspec timer = {0};

    const bool down = state == WL_KEYBOARD_KEY_STATE_PRESSED;

    if (down)
    {
        const xkb_keycode_t keycode = scancode + 8;

        if (xkb_keymap_key_repeats(GetInstance().xkb_.keymap, keycode) && GetInstance().keyRepeatRate_ > 0)
        {
            GetInstance().keyRepeatScancode_ = scancode;
            if (GetInstance().keyRepeatRate_ > 1)
                timer.it_interval.tv_nsec = 1000000000 / GetInstance().keyRepeatRate_;
            else
                timer.it_interval.tv_sec = 1;

            timer.it_value.tv_sec = GetInstance().keyRepeatDelay_ / 1000;
            timer.it_value.tv_nsec = (GetInstance().keyRepeatDelay_ % 1000) * 1000000;
        }
    }

    window->ProcessKeyEvent(TranslateKey(scancode), down);
}

void LinuxWaylandState::HandleKeyboardModifiers(
    void* userData,
    wl_keyboard* keyboard,
    uint32_t serial,
    uint32_t modsDepressed,
    uint32_t modsLatched,
    uint32_t modsLocked,
    uint32_t group)
{
    GetInstance().serial_ = serial;

    if (!GetInstance().xkb_.keymap)
        return;

    xkb_state_update_mask(GetInstance().xkb_.state, modsDepressed, modsLatched, modsLocked, 0, 0, group);

    // TODO: LLGL doesn't support modifiers?

    // GetInstance().xkb_.modifiers = 0;

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

void LinuxWaylandState::HandleKeyboardRepeatInfo(
    void* userData,
    wl_keyboard* keyboard,
    int32_t rate,
    int32_t delay)
{
    if (keyboard != GetInstance().keyboard_)
        return;

    GetInstance().keyRepeatRate_ = rate;
    GetInstance().keyRepeatDelay_ = delay;
}

void LinuxWaylandState::HandleKeyboardKeymap(
    void*           userData,
    wl_keyboard*    keyboard,
    uint32_t        format,
    int             fd,
    uint32_t        size)
{
    xkb_keymap* keymap;
    xkb_state* state;
    xkb_compose_table* composeTable;
    xkb_compose_state* composeState;

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

    keymap = xkb_keymap_new_from_string(GetInstance().xkb_.context,
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

    composeTable = xkb_compose_table_new_from_locale(GetInstance().xkb_.context, locale, XKB_COMPOSE_COMPILE_NO_FLAGS);
    if (composeTable)
    {
        composeState = xkb_compose_state_new(composeTable, XKB_COMPOSE_STATE_NO_FLAGS);
        xkb_compose_table_unref(composeTable);
        if (composeState)
            GetInstance().xkb_.composeState = composeState;
        else
            LLGL_TRAP("Failed to create XKB compose state");
    }
    else
    {
        LLGL_TRAP("Failed to create XKB compose table");
    }

    xkb_keymap_unref(GetInstance().xkb_.keymap);
    xkb_state_unref(GetInstance().xkb_.state);
    GetInstance().xkb_.keymap = keymap;
    GetInstance().xkb_.state = state;

    GetInstance().xkb_.controlIndex  = xkb_keymap_mod_get_index(GetInstance().xkb_.keymap, "Control");
    GetInstance().xkb_.altIndex      = xkb_keymap_mod_get_index(GetInstance().xkb_.keymap, "Mod1");
    GetInstance().xkb_.shiftIndex    = xkb_keymap_mod_get_index(GetInstance().xkb_.keymap, "Shift");
    GetInstance().xkb_.superIndex    = xkb_keymap_mod_get_index(GetInstance().xkb_.keymap, "Mod4");
    GetInstance().xkb_.capsLockIndex = xkb_keymap_mod_get_index(GetInstance().xkb_.keymap, "Lock");
    GetInstance().xkb_.numLockIndex  = xkb_keymap_mod_get_index(GetInstance().xkb_.keymap, "Mod2");
}

#endif

void LinuxWaylandState::HandleOutputGeometry(
    void* userData,
    wl_output* output,
    int32_t x,
    int32_t y,
    int32_t physicalWidth,
    int32_t physicalHeight,
    int32_t subpixel,
    const char* make,
    const char* model,
    int32_t transform)
{
    WaylandDisplayData* display = static_cast<WaylandDisplayData*>(userData);

    display->x = x;
    display->y = y;
    display->widthMM = physicalWidth;
    display->heightMM = physicalHeight;

    if (strlen(display->deviceName) == 0)
        snprintf(display->deviceName, sizeof(display->deviceName), "%s %s", make, model);
}

void LinuxWaylandState::HandleOutputMode(
    void* userData,
    wl_output* output,
    uint32_t flags,
    int32_t width,
    int32_t height,
    int32_t refresh)
{
    WaylandDisplayData* display = static_cast<WaylandDisplayData*>(userData);

    DisplayMode mode;
    mode.resolution = LLGL::Extent2D(width, height);
    mode.refreshRate = (int) std::round(refresh / 1000.0f);

    display->displayModes.push_back(mode);

    if (flags & WL_OUTPUT_MODE_CURRENT)
        display->currentdisplayMode = display->displayModes.size() - 1;
}

void LinuxWaylandState::HandleOutputDone(void* userData, wl_output* output)
{
    WaylandDisplayData* monitor = static_cast<WaylandDisplayData*>(userData);

    if (monitor->widthMM <= 0 || monitor->heightMM <= 0)
    {
        // If Wayland does not provide a physical size, assume the default 96 DPI
        const DisplayMode& mode = monitor->displayModes[monitor->currentdisplayMode];
        monitor->widthMM  = (int) (mode.resolution.width * 25.4f / 96.f);
        monitor->heightMM = (int) (mode.resolution.height * 25.4f / 96.f);
    }
}

void LinuxWaylandState::HandleOutputScale(
    void*         userData,
    wl_output*    output,
    int32_t       factor)
{
    WaylandDisplayData* monitor = static_cast<WaylandDisplayData*>(userData);

    monitor->scale = factor;
}

void LinuxWaylandState::HandleOutputName(void* userData, wl_output* wl_output, const char* name)
{
    WaylandDisplayData* monitor = static_cast<WaylandDisplayData*>(userData);

    strncpy(monitor->deviceName, name, sizeof(monitor->deviceName) - 1);
}

void LinuxWaylandState::HandleOutputDescription(void* userData, wl_output* wl_output, const char* description)
{
}

LinuxWaylandState& LinuxWaylandState::GetInstance()
{
    static LinuxWaylandState instance;
    if (!instance.initialized_)
        instance.Init();
    return instance;
}

#if LLGL_WINDOWING_ENABLED

void LinuxWaylandState::AddWindow(LinuxWindowWayland *window)
{
    GetInstance().windowList_.push_back(window);
}

void LinuxWaylandState::RemoveWindow(LinuxWindowWayland *window)
{
    LinuxWaylandState& instance = GetInstance();

    for (auto it = instance.windowList_.begin(); it != instance.windowList_.end(); ++it)
    {
        if ((*it) == window)
        {
            instance.windowList_.erase(it);
            break;
        }
    }
}

#endif

LinuxWaylandState::~LinuxWaylandState()
{
    for (LinuxDisplayWayland* display : displayList_) {
        wl_output_destroy(display->GetData().output);
    }

    displayList_.clear();

    #if LLGL_WINDOWING_ENABLED

    if (libdecor_.context)
    {
        // Allow libdecor to finish receiving all its requested globals
        // and ensure the associated sync callback object is destroyed
        while (!libdecor_.ready)
            LinuxWaylandState::HandleWaylandEvents(nullptr);

        libdecor_unref(libdecor_.context);
    }

    #endif

    #if LLGL_WINDOWING_ENABLED

    if (xkb_.keymap)
        xkb_keymap_unref(xkb_.keymap);
    if (xkb_.state)
        xkb_state_unref(xkb_.state);
    if (xkb_.context)
        xkb_context_unref(xkb_.context);

    if (subcompositor_)
        wl_subcompositor_destroy(subcompositor_);
    if (compositor_)
        wl_compositor_destroy(compositor_);
    if (shm_)
        wl_shm_destroy(shm_);
    if (viewporter_)
        wp_viewporter_destroy(viewporter_);
    if (decorationManager_)
        zxdg_decoration_manager_v1_destroy(decorationManager_);
    if (xdgWmBase_)
        xdg_wm_base_destroy(xdgWmBase_);
    if (pointer_)
        wl_pointer_destroy(pointer_);
    if (keyboard_)
        wl_keyboard_destroy(keyboard_);
    if (seat_)
        wl_seat_destroy(seat_);

    #endif

    if (registry_)
        wl_registry_destroy(registry_);
    if (display_)
    {
        wl_display_flush(display_);
        wl_display_disconnect(display_);
    }

    #if LLGL_WINDOWING_ENABLED

    if (keyRepeatTimerfd_ >= 0)
        close(keyRepeatTimerfd_);

    #endif
}

void LinuxWaylandState::Init()
{
    initialized_ = true;

    #if LLGL_WINDOWING_ENABLED

    InitKeyTables();

    xkb_.context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!xkb_.context)
    {
        LLGL_TRAP("Failed to initialize xkb context");
    }

    #endif

    display_ = wl_display_connect(nullptr);
    if (!display_)
        LLGL_TRAP("Failed to connect to the Wayland display");

    registry_ = wl_display_get_registry(display_);

    wl_registry_add_listener(registry_, &registryListener_, this);

    wl_display_roundtrip(display_);
    wl_display_roundtrip(display_);

    #if LLGL_WINDOWING_ENABLED

    libdecor_.context = libdecor_new(display_, &LIBDECOR_INTERFACE);
    if (libdecor_.context)
    {
        // Perform an initial dispatch and flush to get the init started
        libdecor_dispatch(libdecor_.context, 0);

        // Create sync point to "know" when libdecor is ready for use
        libdecor_.callback = wl_display_sync(display_);
        wl_callback_add_listener(libdecor_.callback, &libdecorReadyListener_, nullptr);
    }

    if (!compositor_)
        LLGL_TRAP("Failed to get Wayland compositor");
    
    #endif

    tag_ = "LLGL";
}

void LinuxWaylandState::AddWaylandOutput(wl_output* output, uint32_t name, uint32_t version)
{
    wl_proxy_set_tag((wl_proxy*) output, &tag_);

    WaylandDisplayData data;
    data.output = output;
    data.name = name;

    LinuxDisplayWayland* display = new LinuxDisplayWayland(data);
    displayList_.push_back(display);

    wl_output_add_listener(output, &outputListener_, &display->GetData());
}

wl_display* LinuxWaylandState::GetDisplay() noexcept
{
    return GetInstance().display_;
}

#if LLGL_WINDOWING_ENABLED

static bool FlushDisplay()
{
    while (wl_display_flush(LinuxWaylandState::GetDisplay()) == -1)
    {
        if (errno != EAGAIN)
            return false;

        pollfd fd = { wl_display_get_fd(LinuxWaylandState::GetDisplay()), POLLOUT };

        while (poll(&fd, 1, -1) == -1)
        {
            if (errno != EINTR && errno != EAGAIN)
                return false;
        }
    }
    return true;
}

static bool PollPosix(pollfd* fds, nfds_t count, double* timeout)
{
    for (;;)
    {
        if (timeout)
        {
            const uint64_t base = Timer::Tick();

#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__CYGWIN__)
            const time_t seconds = (time_t) *timeout;
            const long nanoseconds = (long) ((*timeout - seconds) * 1e9);
            const timespec ts = { seconds, nanoseconds };
            const int result = ppoll(fds, count, &ts, NULL);
#elif defined(__NetBSD__)
            const time_t seconds = (time_t) *timeout;
            const long nanoseconds = (long) ((*timeout - seconds) * 1e9);
            const timespec ts = { seconds, nanoseconds };
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

void LinuxWaylandState::HandleWaylandEventsInternal(double* timeout)
{
    bool event = false;

    enum { DISPLAY_FD, KEYREPEAT_FD, LIBDECOR_FD };
    pollfd fds[3];
    fds[DISPLAY_FD] = { wl_display_get_fd(display_), POLLIN };
    fds[KEYREPEAT_FD] = { keyRepeatTimerfd_, POLLIN };
    fds[LIBDECOR_FD] = { -1, POLLIN };

    if (libdecor_.context)
        fds[LIBDECOR_FD].fd = libdecor_get_fd(libdecor_.context);

    while (!event)
    {
        while (wl_display_prepare_read(display_) != 0)
        {
            if (wl_display_dispatch_pending(display_) > 0)
                return;
        }

        // If an error other than EAGAIN happens, we have likely been disconnected
        // from the Wayland session; try to handle that the best we can.
        if (!FlushDisplay())
        {
            wl_display_cancel_read(display_);

            for (LinuxWindowWayland* window : GetWindowList())
            {
                window->GetState().shouldClose = true;
            }

            return;
        }

        if (!PollPosix(fds, sizeof(fds) / sizeof(fds[0]), timeout))
        {
            wl_display_cancel_read(display_);
            return;
        }

        if (fds[DISPLAY_FD].revents & POLLIN)
        {
            wl_display_read_events(display_);
            if (wl_display_dispatch_pending(display_) > 0)
                event = true;
        }
        else
            wl_display_cancel_read(display_);

        if (fds[KEYREPEAT_FD].revents & POLLIN)
        {
            uint64_t repeats;

            if (read(keyRepeatTimerfd_, &repeats, sizeof(repeats)) == 8)
            {
                for (uint64_t i = 0; i < repeats; i++)
                {
                    keyboardFocus_->ProcessKeyEvent(TranslateKey(keyRepeatScancode_), true);
                }

                event = true;
            }
        }

        if (fds[LIBDECOR_FD].revents & POLLIN)
        {
            if (libdecor_dispatch(libdecor_.context, 0) > 0)
                event = true;
        }
    }
}

void LinuxWaylandState::InitKeyTables()
{
    memset(keycodes_, static_cast<int>(Key::Any), sizeof(keycodes_));

    keycodes_[KEY_1]          = Key::D1;
    keycodes_[KEY_2]          = Key::D2;
    keycodes_[KEY_3]          = Key::D3;
    keycodes_[KEY_4]          = Key::D4;
    keycodes_[KEY_5]          = Key::D5;
    keycodes_[KEY_6]          = Key::D6;
    keycodes_[KEY_7]          = Key::D7;
    keycodes_[KEY_8]          = Key::D8;
    keycodes_[KEY_9]          = Key::D9;
    keycodes_[KEY_0]          = Key::D0;
    keycodes_[KEY_SPACE]      = Key::Space;
    keycodes_[KEY_MINUS]      = Key::Minus;
    keycodes_[KEY_Q]          = Key::Q;
    keycodes_[KEY_W]          = Key::W;
    keycodes_[KEY_E]          = Key::E;
    keycodes_[KEY_R]          = Key::R;
    keycodes_[KEY_T]          = Key::T;
    keycodes_[KEY_Y]          = Key::Y;
    keycodes_[KEY_U]          = Key::U;
    keycodes_[KEY_I]          = Key::I;
    keycodes_[KEY_O]          = Key::O;
    keycodes_[KEY_P]          = Key::P;
    // keycodes_[KEY_LEFTBRACE]  = Key::LeftBracket;
    // keycodes_[KEY_RIGHTBRACE] = Key::RightBracket;
    keycodes_[KEY_A]          = Key::A;
    keycodes_[KEY_S]          = Key::S;
    keycodes_[KEY_D]          = Key::D;
    keycodes_[KEY_F]          = Key::F;
    keycodes_[KEY_G]          = Key::G;
    keycodes_[KEY_H]          = Key::H;
    keycodes_[KEY_J]          = Key::J;
    keycodes_[KEY_K]          = Key::K;
    keycodes_[KEY_L]          = Key::L;
    // keycodes_[KEY_SEMICOLON]  = Key::Semicolon;
    // keycodes_[KEY_APOSTROPHE] = Key::Apostrophe;
    keycodes_[KEY_Z]          = Key::Z;
    keycodes_[KEY_X]          = Key::X;
    keycodes_[KEY_C]          = Key::C;
    keycodes_[KEY_V]          = Key::V;
    keycodes_[KEY_B]          = Key::B;
    keycodes_[KEY_N]          = Key::N;
    keycodes_[KEY_M]          = Key::M;
    keycodes_[KEY_COMMA]      = Key::Comma;
    keycodes_[KEY_DOT]        = Key::Period;
    // keycodes_[KEY_SLASH]      = Key::Slash;
    // keycodes_[KEY_BACKSLASH]  = Key::Backslash;
    keycodes_[KEY_ESC]        = Key::Escape;
    keycodes_[KEY_TAB]        = Key::Tab;
    keycodes_[KEY_LEFTSHIFT]  = Key::LShift;
    keycodes_[KEY_RIGHTSHIFT] = Key::RShift;
    keycodes_[KEY_LEFTCTRL]   = Key::LControl;
    keycodes_[KEY_RIGHTCTRL]  = Key::RControl;
    // keycodes_[KEY_LEFTALT]    = Key::LAlt;
    // keycodes_[KEY_RIGHTALT]   = Key::RAlt;
    keycodes_[KEY_LEFTMETA]   = Key::LWin;
    keycodes_[KEY_RIGHTMETA]  = Key::RWin;
    keycodes_[KEY_NUMLOCK]    = Key::NumLock;
    // keycodes_[KEY_CAPSLOCK]   = Key::CapsLock;
    keycodes_[KEY_PRINT]      = Key::Print;
    keycodes_[KEY_SCROLLLOCK] = Key::ScrollLock;
    keycodes_[KEY_PAUSE]      = Key::Pause;
    keycodes_[KEY_DELETE]     = Key::Delete;
    keycodes_[KEY_BACKSPACE]  = Key::Back;
    keycodes_[KEY_ENTER]      = Key::Return;
    keycodes_[KEY_HOME]       = Key::Home;
    keycodes_[KEY_END]        = Key::End;
    keycodes_[KEY_PAGEUP]     = Key::PageUp;
    keycodes_[KEY_PAGEDOWN]   = Key::PageDown;
    keycodes_[KEY_INSERT]     = Key::Insert;
    keycodes_[KEY_LEFT]       = Key::Left;
    keycodes_[KEY_RIGHT]      = Key::Right;
    keycodes_[KEY_DOWN]       = Key::Down;
    keycodes_[KEY_UP]         = Key::Up;
    keycodes_[KEY_F1]         = Key::F1;
    keycodes_[KEY_F2]         = Key::F2;
    keycodes_[KEY_F3]         = Key::F3;
    keycodes_[KEY_F4]         = Key::F4;
    keycodes_[KEY_F5]         = Key::F5;
    keycodes_[KEY_F6]         = Key::F6;
    keycodes_[KEY_F7]         = Key::F7;
    keycodes_[KEY_F8]         = Key::F8;
    keycodes_[KEY_F9]         = Key::F9;
    keycodes_[KEY_F10]        = Key::F10;
    keycodes_[KEY_F11]        = Key::F11;
    keycodes_[KEY_F12]        = Key::F12;
    keycodes_[KEY_F13]        = Key::F13;
    keycodes_[KEY_F14]        = Key::F14;
    keycodes_[KEY_F15]        = Key::F15;
    keycodes_[KEY_F16]        = Key::F16;
    keycodes_[KEY_F17]        = Key::F17;
    keycodes_[KEY_F18]        = Key::F18;
    keycodes_[KEY_F19]        = Key::F19;
    keycodes_[KEY_F20]        = Key::F20;
    keycodes_[KEY_F21]        = Key::F21;
    keycodes_[KEY_F22]        = Key::F22;
    keycodes_[KEY_F23]        = Key::F23;
    keycodes_[KEY_F24]        = Key::F24;
    keycodes_[KEY_KPSLASH]    = Key::KeypadDivide;
    keycodes_[KEY_KPASTERISK] = Key::KeypadMultiply;
    keycodes_[KEY_KPMINUS]    = Key::KeypadMinus;
    keycodes_[KEY_KPPLUS]     = Key::KeypadPlus;
    keycodes_[KEY_KP0]        = Key::Keypad0;
    keycodes_[KEY_KP1]        = Key::Keypad1;
    keycodes_[KEY_KP2]        = Key::Keypad2;
    keycodes_[KEY_KP3]        = Key::Keypad3;
    keycodes_[KEY_KP4]        = Key::Keypad4;
    keycodes_[KEY_KP5]        = Key::Keypad5;
    keycodes_[KEY_KP6]        = Key::Keypad6;
    keycodes_[KEY_KP7]        = Key::Keypad7;
    keycodes_[KEY_KP8]        = Key::Keypad8;
    keycodes_[KEY_KP9]        = Key::Keypad9;
    keycodes_[KEY_KPDOT]      = Key::KeypadDecimal;
    // keycodes_[KEY_KPEQUAL]    = Key::KeypadEqual;
    // keycodes_[KEY_KPENTER]    = Key::KeypadEnter;
}

void LinuxWaylandState::HandleWaylandEvents(double* timeout) {
    return GetInstance().HandleWaylandEventsInternal(timeout);
}

wl_compositor* LinuxWaylandState::GetCompositor() noexcept
{
    return GetInstance().compositor_;
}

wl_subcompositor* LinuxWaylandState::GetSubcompositor() noexcept
{
    return GetInstance().subcompositor_;
}

wl_seat* LinuxWaylandState::GetSeat() noexcept
{
    return GetInstance().seat_;
}

wp_viewporter* LinuxWaylandState::GetViewporter() noexcept
{
    return GetInstance().viewporter_;
}

wl_shm* LinuxWaylandState::GetShm() noexcept
{
    return GetInstance().shm_;
}

xdg_wm_base* LinuxWaylandState::GetXdgWmBase() noexcept
{
    return GetInstance().xdgWmBase_;
}

zxdg_decoration_manager_v1* LinuxWaylandState::GetDecorationManager() noexcept
{
    return GetInstance().decorationManager_;
}

const char *const * LinuxWaylandState::GetTag() noexcept
{
    return &GetInstance().tag_;
}

const XkbContext& LinuxWaylandState::GetXkb() noexcept
{
    return GetInstance().xkb_;
}

const LibdecorContext& LinuxWaylandState::GetLibdecor() noexcept
{
    return GetInstance().libdecor_;
}

LLGL::ArrayView<Key> LinuxWaylandState::GetKeycodes() noexcept
{
    return GetInstance().keycodes_;
}

const LLGL::DynamicVector<LinuxWindowWayland*>& LinuxWaylandState::GetWindowList() noexcept
{
    return GetInstance().windowList_;
}

#endif

const LLGL::DynamicVector<LinuxDisplayWayland*>& LinuxWaylandState::GetDisplayList() noexcept
{
    return GetInstance().displayList_;
}

}

#endif