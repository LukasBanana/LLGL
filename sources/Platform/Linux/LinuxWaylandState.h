/*
 * LinuxWaylandState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_WAYLAND_STATE_H
#define LLGL_LINUX_WAYLAND_STATE_H

#if LLGL_LINUX_ENABLE_WAYLAND

#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <libdecor-0/libdecor.h>

#include <LLGL/Key.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/DynamicVector.h>

#if LLGL_WINDOWING_ENABLED
#   include "LinuxWindowWayland.h"
#endif

#include "LinuxDisplayWayland.h"

#include "protocols/xdg-shell-client-protocol.h"

struct wp_viewporter;
struct zxdg_decoration_manager_v1;

namespace LLGL {

struct XkbContext
{
    xkb_context*          context = nullptr;
    xkb_keymap*           keymap = nullptr;
    xkb_state*            state = nullptr;

    xkb_compose_state*    composeState = nullptr;

    xkb_mod_index_t       controlIndex = 0;
    xkb_mod_index_t       altIndex = 0;
    xkb_mod_index_t       shiftIndex = 0;
    xkb_mod_index_t       superIndex = 0;
    xkb_mod_index_t       capsLockIndex = 0;
    xkb_mod_index_t       numLockIndex = 0;
    unsigned int          modifiers = 0;
};

struct LibdecorContext
{
    libdecor*       context  = nullptr;
    wl_callback*    callback = nullptr;
    bool            ready    = false;
};

class LinuxWaylandState
{

    public:

        ~LinuxWaylandState();

        #if LLGL_WINDOWING_ENABLED

        static void HandleWaylandEvents(double* timeout);

        #endif

        static wl_display* GetDisplay() noexcept;
        static wl_compositor* GetCompositor() noexcept;
        static wl_subcompositor* GetSubcompositor() noexcept;

        #if LLGL_WINDOWING_ENABLED

        static wl_seat* GetSeat() noexcept;
        static wp_viewporter* GetViewporter() noexcept;
        static wl_shm* GetShm() noexcept;
        static xdg_wm_base* GetXdgWmBase() noexcept;
        static zxdg_decoration_manager_v1* GetDecorationManager() noexcept;
        static const XkbContext& GetXkb() noexcept;
        static const LibdecorContext& GetLibdecor() noexcept;
        static LLGL::ArrayView<Key> GetKeycodes() noexcept;

        static const LLGL::DynamicVector<LinuxWindowWayland*>& GetWindowList() noexcept;

        static void AddWindow(LinuxWindowWayland* window);
        static void RemoveWindow(LinuxWindowWayland* window);

        #endif

        static const char *const * GetTag() noexcept;

        static const LLGL::DynamicVector<LinuxDisplayWayland*>& GetDisplayList() noexcept;

    private:

        static LinuxWaylandState& GetInstance();

        void HandleWaylandEventsInternal(double* timeout);

        void Init();

        void AddWaylandOutput(wl_output* output, uint32_t name, uint32_t version);

        #if LLGL_WINDOWING_ENABLED

        static Key TranslateKey(uint32_t scancode);

        void InitKeyTables();

        #endif

        static void HandleRegistryGlobal(void* userData, wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
        static void HandleRegistryRemove(void* userData, wl_registry* registry, uint32_t name);

        static void HandleXdgWmBasePing(void* userData, xdg_wm_base* xdg_wm_base, uint32_t serial);

        #if LLGL_WINDOWING_ENABLED

        static void HandleSeatCapabilities(void* userData, wl_seat* seat, uint32_t caps);
        static void HandleSeatName(void* userData, wl_seat* seat, const char* name);

        static void HandlePointerEnter(void* userData, wl_pointer* pointer, uint32_t serial, wl_surface* surface, wl_fixed_t surface_x, wl_fixed_t surface_y);
        static void HandlePointerLeave(void* userData, wl_pointer* pointer, uint32_t serial, wl_surface* surface);
        static void HandlePointerMotion(void* userData, wl_pointer* pointer,uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y);
        static void HandlePointerButton(void* userData, wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
        static void HandlePointerAxis(void* userData, wl_pointer* pointer, uint32_t time, uint32_t axis, wl_fixed_t value);

        static void HandleKeyboardKeymap(void* userData, wl_keyboard* keyboard, uint32_t format, int fd, uint32_t size);
        static void HandleKeyboardEnter(void* userData, wl_keyboard* keyboard, uint32_t serial, wl_surface* surface, wl_array* keys);
        static void HandleKeyboardLeave(void* userData, wl_keyboard* keyboard, uint32_t serial, wl_surface* surface);
        static void HandleKeyboardKey(void* userData, wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t scancode, uint32_t state);
        static void HandleKeyboardModifiers(void* userData, wl_keyboard* keyboard, uint32_t serial, uint32_t modsDepressed, uint32_t modsLatched, uint32_t modsLocked, uint32_t group);
        static void HandleKeyboardRepeatInfo(void* userData, wl_keyboard* keyboard, int32_t rate, int32_t delay);

        #endif

        static void HandleOutputGeometry(void* userData, wl_output* output, int32_t x, int32_t y, int32_t physicalWidth, int32_t physicalHeight, int32_t subpixel, const char* make, const char* model, int32_t transform);
        static void HandleOutputMode(void* userData, wl_output* output, uint32_t flags, int32_t width, int32_t height, int32_t refresh);
        static void HandleOutputDone(void* userData, wl_output* output);
        static void HandleOutputScale(void* userData, wl_output* output, int32_t factor);
        static void HandleOutputName(void* userData, wl_output* wl_output, const char* name);
        static void HandleOutputDescription(void* userData, wl_output* wl_output, const char* description);

        static void HandleLibdecorReady(void* userData, wl_callback* callback, uint32_t time);

    private:

        const char*          tag_           = nullptr;

        wl_registry*         registry_      = nullptr;
        wl_display*          display_       = nullptr;
        wl_compositor*       compositor_    = nullptr;
        wl_subcompositor*    subcompositor_ = nullptr;

        #if LLGL_WINDOWING_ENABLED

        wl_seat*             seat_          = nullptr;
        wp_viewporter*       viewporter_    = nullptr;
        wl_shm*              shm_           = nullptr;

        zxdg_decoration_manager_v1* decorationManager_ = nullptr;

        wl_pointer*            pointer_            = nullptr;
        LinuxWindowWayland*    pointerFocus_       = nullptr;
        uint32_t               serial_             = 0;
        uint32_t               pointerEnterSerial_ = 0;

        wl_keyboard*        keyboard_      = nullptr;
        LinuxWindowWayland* keyboardFocus_ = nullptr;

        xdg_wm_base *xdgWmBase_ = nullptr;


        int keyRepeatTimerfd_ = 0;
        int keyRepeatRate_ = 0;
        int keyRepeatDelay_ = 0;
        int keyRepeatScancode_ = 0;

        Key keycodes_[256] = {};

        XkbContext xkb_;

        LibdecorContext libdecor_;

        DynamicVector<LinuxWindowWayland*> windowList_;

        #endif

        DynamicVector<LinuxDisplayWayland*> displayList_;


        bool initialized_ = false;

    private:

        static wl_registry_listener registryListener_;

        #if LLGL_WINDOWING_ENABLED
        static wl_seat_listener seatListener_;
        static wl_pointer_listener pointerListener_;
        static wl_keyboard_listener keyboardListener_;
        static wl_callback_listener libdecorReadyListener_;
        static xdg_wm_base_listener xdgWmBaseListener_;
        #endif

        static wl_output_listener outputListener_;

        static LinuxWaylandState* instance_;
};

} // /namespace LLGL

#endif // LLGL_LINUX_ENABLE_WAYLAND

#endif // LLGL_LINUX_WAYLAND_STATE_H