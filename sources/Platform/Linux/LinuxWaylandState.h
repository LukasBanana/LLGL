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

#include "LinuxWindowWayland.h"
#include "LinuxDisplayWayland.h"

#include "protocols/xdg-shell-client-protocol.h"

struct xdg_wm_base;
struct wp_viewporter;
struct zxdg_decoration_manager_v1;

namespace LLGL {

struct XkbContext
{
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
};

struct LibdecorContext
{
    struct libdecor* context;
    struct wl_callback* callback;
    bool ready = false;
};

class LinuxWaylandState
{

    public:

        LinuxWaylandState();
        ~LinuxWaylandState();

        static void HandleWaylandEvents(double* timeout);
        static wl_display* GetDisplay() noexcept;
        static wl_compositor* GetCompositor() noexcept;
        static wl_subcompositor* GetSubcompositor() noexcept;
        static wl_seat* GetSeat() noexcept;
        static wp_viewporter* GetViewporter() noexcept;
        static wl_shm* GetShm() noexcept;
        static xdg_wm_base* GetXdgWmBase() noexcept;
        static zxdg_decoration_manager_v1* GetDecorationManager() noexcept;
        static const char *const * GetTag() noexcept;
        static const XkbContext& GetXkb() noexcept;
        static const LibdecorContext& GetLibdecor() noexcept;
        static LLGL::ArrayView<Key> GetKeycodes() noexcept;
        static const LLGL::DynamicVector<LinuxDisplayWayland*>& GetDisplayList() noexcept;

        void BindPointerListener(const wl_pointer_listener& listener) noexcept
        {
            pointerListener_ = listener;
        }

        void BindKeyboardListener(const wl_keyboard_listener& listener) noexcept
        {
            keyboardListener_.enter = listener.enter;
            keyboardListener_.leave = listener.leave;
            keyboardListener_.key = listener.key;
            keyboardListener_.modifiers = listener.modifiers;
            keyboardListener_.repeat_info = listener.repeat_info;
        }

    private:

        static LinuxWaylandState& GetInstance();

        void HandleWaylandEventsInternal(double* timeout);

        void Init();

        void InitKeyTables();
        void AddWaylandOutput(wl_output* output, uint32_t name, uint32_t version);

        static Key TranslateKey(uint32_t scancode);

        static void HandleRegistryGlobal(void* userData, wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
        static void HandleRegistryRemove(void* userData, wl_registry* registry, uint32_t name);

        static void HandleXdgWmBasePing(void* userData, xdg_wm_base* xdg_wm_base, uint32_t serial);

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

        static void HandleOutputGeometry(void* userData, wl_output* output, int32_t x, int32_t y, int32_t physicalWidth, int32_t physicalHeight, int32_t subpixel, const char* make, const char* model, int32_t transform);
        static void HandleOutputMode(void* userData, wl_output* output, uint32_t flags, int32_t width, int32_t height, int32_t refresh);
        static void HandleOutputDone(void* userData, wl_output* output);
        static void HandleOutputScale(void* userData, wl_output* output, int32_t factor);
        static void HandleOutputName(void* userData, wl_output* wl_output, const char* name);
        static void HandleOutputDescription(void* userData, struct wl_output* wl_output, const char* description);

        static void HandleLibdecorReady(void* userData, wl_callback* callback, uint32_t time);

    private:

        wl_display* display_;
        wl_compositor* compositor_;
        wl_subcompositor* subcompositor_;
        wl_seat* seat_;
        wp_viewporter* viewporter_;
        wl_shm* shm_;

        zxdg_decoration_manager_v1* decorationManager_;

        wl_pointer* pointer_;
        LinuxWindowWayland* pointerFocus_;
        uint32_t serial_;
        uint32_t pointerEnterSerial_;

        wl_keyboard* keyboard_;
        LinuxWindowWayland* keyboardFocus_;

        struct xdg_wm_base *xdg_wm_base_;

        const char* tag_;

        int keyRepeatTimerfd_;
        int keyRepeatRate_;
        int keyRepeatDelay_;
        int keyRepeatScancode_;

        Key keycodes_[256];

        XkbContext xkb_;

        LibdecorContext libdecor_;

        LLGL::DynamicVector<LLGL::LinuxDisplayWayland*> displayList_;

        bool initialized_ = false;

    private:

        static wl_registry_listener registryListener_;
        static wl_seat_listener seatListener_;
        static wl_pointer_listener pointerListener_;
        static wl_keyboard_listener keyboardListener_;
        static wl_callback_listener libdecorReadyListener_;
        static wl_output_listener outputListener_;
        static xdg_wm_base_listener xdgWmBaseListener_;

        static LinuxWaylandState* instance_;
};

} // /namespace LLGL

#endif // LLGL_LINUX_ENABLE_WAYLAND

#endif // LLGL_LINUX_WAYLAND_STATE_H