/*
 * LinuxWindowWayland.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Display.h>
#include <LLGL/Timer.h>
#include <LLGL/Log.h>
#include <LLGL/Utils/ForRange.h>

#include "../../../Core/Exception.h"
#include "../../../Core/Assertion.h"

#include <unistd.h>
#include <string.h>
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

#include "LinuxWindowWayland.h"
#include "LinuxDisplayWayland.h"
#include "LinuxWaylandState.h"


namespace LLGL
{


static constexpr int k_DecorationBorerSize      = 4;
static constexpr int k_DecorationCaptionHeight  = 24;

/*
 * ======= SURFACE EVENTS =======
 */

static void WaylandSurfaceHandleEnterCallback(void* userData, wl_surface* surface, wl_output* output)
{
    //TODO
}

static void WaylandSurfaceHandleLeaveCallback(void* userData, wl_surface* surface, wl_output* output)
{
    //TODO
}

static const wl_surface_listener g_WaylandSurfaceListener =
{
    WaylandSurfaceHandleEnterCallback,
    WaylandSurfaceHandleLeaveCallback
};


/*
 * ======= XDG SURFACE EVENTS =======
 */

static void XDGSurfaceConfigureCallback(
    void*           userData,
    xdg_surface*    xdg_surface,
    uint32_t        serial)
{
    xdg_surface_ack_configure(xdg_surface, serial);
}

static const xdg_surface_listener g_XDGSurfaceListener =
{
    .configure = XDGSurfaceConfigureCallback,
};


/*
 * ======= XDG TOPLEVEL EVENTS =======
 */

static void XDGTopLevelHandleConfigureCallback(
    void*           userData,
    xdg_toplevel*   xdg_toplevel,
    int32_t         width,
    int32_t         height,
    wl_array*       states)
{
	LinuxWindowWayland* window = static_cast<LinuxWindowWayland*>(userData);
    LLGL_ASSERT(width >= 0, "Width is negative");
    LLGL_ASSERT(height >= 0, "Height is negative");

    LinuxWindowWayland::State& state = window->GetState();

	if (width != 0 && height != 0 && (width != state.size.width || height != state.size.height))
        window->SetSizeInternal(Extent2D{ width, height });
}

static void XDGTopLevelHandleCloseCallback(void* userData, xdg_toplevel* toplevel)
{
	LinuxWindowWayland* window = static_cast<LinuxWindowWayland*>(userData);
    window->GetState().shouldClose = true;
}

static const xdg_toplevel_listener g_XDGTopLevelListener =
{
	.configure  = XDGTopLevelHandleConfigureCallback,
	.close      = XDGTopLevelHandleCloseCallback,
};

static void WaylandSetContentAreaOpaque(LinuxWindowWayland::State& state)
{
    if (wl_region* region = wl_compositor_create_region(LinuxWaylandState::GetCompositor()))
    {
        wl_region_add(region, 0, 0, state.size.width, state.size.height);
        wl_surface_set_opaque_region(state.wl.surface, region);
        wl_region_destroy(region);
    }
}

static void WaylandResizeFramebuffer(LinuxWindowWayland::State& state)
{
    state.framebufferSize.width = state.size.width * state.framebufferScale;
    state.framebufferSize.height = state.size.height * state.framebufferScale;
    WaylandSetContentAreaOpaque(state);
}

static bool WaylandResizeWindow(LinuxWindowWayland& window, int width, int height)
{
    LinuxWindowWayland::State& state = window.GetState();

    width = std::max(width, 1);
    height = std::max(height, 1);

    if (width == state.size.width && height == state.size.height)
        return false;

    window.SetSizeInternal(Extent2D(width, height));

    WaylandResizeFramebuffer(state);

    // if (state.scalingViewport)
    // {
    //     wp_viewport_set_destination(state.scalingViewport,
    //                                 state.size.width,
    //                                 state.size.height);
    // }

    if (state.fallback.decorations)
    {
        wp_viewport_set_destination(
            state.fallback.top.viewport,
            state.size.width,
            k_DecorationCaptionHeight
        );
        wl_surface_commit(state.fallback.top.surface);

        wp_viewport_set_destination(
            state.fallback.left.viewport,
            k_DecorationBorerSize,
            state.size.height + k_DecorationCaptionHeight
        );
        wl_surface_commit(state.fallback.left.surface);

        wl_subsurface_set_position(
            state.fallback.right.subsurface,
            state.size.width,
            -k_DecorationCaptionHeight
        );
        wp_viewport_set_destination(
            state.fallback.right.viewport,
            k_DecorationBorerSize,
            state.size.height + k_DecorationCaptionHeight
        );
        wl_surface_commit(state.fallback.right.surface);

        wl_subsurface_set_position(
            state.fallback.bottom.subsurface,
            -k_DecorationBorerSize,
            state.size.height
        );
        wp_viewport_set_destination(
            state.fallback.bottom.viewport,
            state.size.width + k_DecorationBorerSize * 2,
            k_DecorationBorerSize
        );
        wl_surface_commit(state.fallback.bottom.surface);
    }

    return true;
}

static void LibdecorFrameHandleConfigureCallback(
    libdecor_frame*         frame,
    libdecor_configuration* config,
    void*                   userData)
{
    LinuxWindowWayland* window = static_cast<LinuxWindowWayland*>(userData);
    LinuxWindowWayland::State& state = window->GetState();

    enum libdecor_window_state windowState;
    bool fullscreen  = false;
    bool activated   = false;
    bool maximized   = false;

    if (libdecor_configuration_get_window_state(config, &windowState))
    {
        fullscreen  = ((windowState & LIBDECOR_WINDOW_STATE_FULLSCREEN) != 0);
        activated   = ((windowState & LIBDECOR_WINDOW_STATE_ACTIVE    ) != 0);
        maximized   = ((windowState & LIBDECOR_WINDOW_STATE_MAXIMIZED ) != 0);
    }
    else
    {
        fullscreen  = state.fullscreen;
        activated   = state.activated;
        maximized   = state.maximized;
    }

    int width = 0, height = 0;
    if (!libdecor_configuration_get_content_size(config, frame, &width, &height))
    {
        width = state.size.width;
        height = state.size.height;
    }

    libdecor_state* frameState = libdecor_state_new(width, height);
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

    if (WaylandResizeWindow(*window, width, height))
        damaged = true;

    if (damaged)
        window->PostUpdate();
    else
        wl_surface_commit(state.wl.surface);
}

static void LibdecorFrameHandleCloseCallback(libdecor_frame* frame, void* userData)
{
    LinuxWindowWayland* window = static_cast<LinuxWindowWayland*>(userData);
    LinuxWindowWayland::State& state = window->GetState();
    state.shouldClose = true;
}

static void LibdecorFrameHandleCommitCallback(libdecor_frame* frame, void* userData)
{
    LinuxWindowWayland* window = static_cast<LinuxWindowWayland*>(userData);
    LinuxWindowWayland::State& state = window->GetState();
    wl_surface_commit(state.wl.surface);
}

static void LibdecorFrameHandleDismissPopupCallback(libdecor_frame* frame, const char* seatName, void* userData)
{
    // dummy
}

static libdecor_frame_interface libdecorFrameInterface =
{
    LibdecorFrameHandleConfigureCallback,
    LibdecorFrameHandleCloseCallback,
    LibdecorFrameHandleCommitCallback,
    LibdecorFrameHandleDismissPopupCallback
};

static bool CreateLibdecorFrame(LinuxWindowWayland& window)
{
    // Allow libdecor to finish initialization of itself and its plugin
    while (!LinuxWaylandState::GetLibdecor().ready)
        LinuxWaylandState::HandleWaylandEvents(nullptr);

    LinuxWindowWayland::State& state = window.GetState();

    state.libdecor.frame = libdecor_decorate(LinuxWaylandState::GetLibdecor().context, state.wl.surface, &libdecorFrameInterface, &window);

    if (!state.libdecor.frame)
    {
        Log::Errorf("Wayland: Failed to create libdecor frame\n");
        return false;
    }

    libdecor_state* frameState = libdecor_state_new(state.size.width, state.size.height);
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
    wl_display_roundtrip(LinuxWaylandState::GetDisplay());
    return true;
}

static void CreateFallbackEdge(
    LinuxWindowWayland& window,
    FallbackEdge*       edge,
    wl_surface*         parent,
    wl_buffer*          buffer,
    int                 x,
    int                 y,
    int                 width,
    int                 height)
{
    edge->surface = wl_compositor_create_surface(LinuxWaylandState::GetCompositor());
    wl_surface_set_user_data(edge->surface, &window);
    wl_proxy_set_tag((wl_proxy*) edge->surface, LinuxWaylandState::GetTag());
    edge->subsurface = wl_subcompositor_get_subsurface(LinuxWaylandState::GetSubcompositor(), edge->surface, parent);
    wl_subsurface_set_position(edge->subsurface, x, y);
    edge->viewport = wp_viewporter_get_viewport(LinuxWaylandState::GetViewporter(), edge->surface);
    wp_viewport_set_destination(edge->viewport, width, height);
    wl_surface_attach(edge->surface, buffer, 0, 0);

    wl_region* region = wl_compositor_create_region(LinuxWaylandState::GetCompositor());
    wl_region_add(region, 0, 0, width, height);
    wl_surface_set_opaque_region(edge->surface, region);
    wl_surface_commit(edge->surface);
    wl_region_destroy(region);
}

static void RandName(char* buf, std::size_t size)
{
    LLGL_ASSERT(size >= 6);
	timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	long r = ts.tv_nsec;
	for_range(i, 6)
    {
		buf[i] = 'A' + (r & 15) + (r & 16)*2;
		r >>= 5;
	}
}

// Creates a uniquely named POSIX shared memory object
static int CreateShmFile()
{
	int retries = 100;
	do
    {
		char name[15];
        strncpy(name, "/wl_shm-XXXXXX", sizeof(name));
		RandName(name + sizeof(name) - 7, 6);
		--retries;
		int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
		if (fd >= 0)
        {
			shm_unlink(name);
			return fd;
		}
	}
    while (retries > 0 && errno == EEXIST);
	return -1;
}

static int AllocateShmFile(std::size_t size)
{
	int fd = CreateShmFile();
	if (fd < 0)
		return -1;

	int ret;
	do
    {
		ret = ftruncate(fd, size);
	}
    while (ret < 0 && errno == EINTR);

	if (ret < 0)
    {
		close(fd);
		return -1;
	}

	return fd;
}

static wl_buffer* CreateShmBuffer(int width, int height, const std::uint8_t* pixels)
{
    const int stride = width * 4;
    const int length = width * height * 4;

    const int fd = AllocateShmFile(length);
    if (fd < 0)
    {
        Log::Errorf("Wayland: Failed to create buffer file of size %d: %s\n", length, strerror(errno));
        return NULL;
    }

    void* data = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
    {
        Log::Errorf("Wayland: Failed to map file: %s\n", strerror(errno));
        close(fd);
        return NULL;
    }

    wl_shm_pool* pool = wl_shm_create_pool(LinuxWaylandState::GetShm(), fd, length);

    close(fd);

    const std::uint8_t* source = pixels;
    std::uint8_t* target = static_cast<std::uint8_t*>(data);

    for_range(i, width * height)
    {
        std::uint32_t alpha = source[3];

        target[0] = static_cast<std::uint8_t>(((source[2] * alpha) / 255));
        target[1] = static_cast<std::uint8_t>(((source[1] * alpha) / 255));
        target[2] = static_cast<std::uint8_t>(((source[0] * alpha) / 255));
        target[3] = static_cast<std::uint8_t>(alpha);

        source += 4;
        target += 4;
    }

    wl_buffer* buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
    munmap(data, length);
    wl_shm_pool_destroy(pool);

    return buffer;
}

static void CreateFallbackDecorations(LinuxWindowWayland& window)
{
    LinuxWindowWayland::State& state = window.GetState();

    if (!LinuxWaylandState::GetViewporter())
        return;

    const std::uint8_t pixel[] = { 224, 224, 224, 255 };
    if (!state.fallback.buffer)
        state.fallback.buffer = CreateShmBuffer(1, 1, pixel);
    if (!state.fallback.buffer)
        return;

    CreateFallbackEdge(
        window,
        &state.fallback.top,
        state.wl.surface,
        state.fallback.buffer,
        0,
        -k_DecorationCaptionHeight,
        state.size.width,
        k_DecorationCaptionHeight
    );
    CreateFallbackEdge(
        window,
        &state.fallback.left,
        state.wl.surface,
        state.fallback.buffer,
        -k_DecorationBorerSize,
        -k_DecorationCaptionHeight,
        k_DecorationBorerSize,
        state.size.height + k_DecorationCaptionHeight
    );
    CreateFallbackEdge(
        window,
        &state.fallback.right,
        state.wl.surface,
        state.fallback.buffer,
        state.size.width,
        -k_DecorationCaptionHeight,
        k_DecorationBorerSize,
        state.size.height + k_DecorationCaptionHeight
    );
    CreateFallbackEdge(
        window,
        &state.fallback.bottom,
        state.wl.surface,
        state.fallback.buffer,
        -k_DecorationBorerSize,
        state.size.height,
        state.size.width + k_DecorationBorerSize * 2,
        k_DecorationBorerSize
    );

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

static void XdgDecorationHandleConfigure(void* userData, zxdg_toplevel_decoration_v1* decoration, uint32_t mode)
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

static const zxdg_toplevel_decoration_v1_listener g_XDGDecorationListener =
{
    XdgDecorationHandleConfigure,
};

static bool CreateXdgShellObjects(LinuxWindowWayland& window)
{
    LinuxWindowWayland::State& state = window.GetState();

    state.xdg.surface = xdg_wm_base_get_xdg_surface(LinuxWaylandState::GetXdgWmBase(), state.wl.surface);
    if (!state.xdg.surface)
    {
        Log::Errorf("Wayland: Failed to create xdg-surface for window\n");
        return false;
    }

    xdg_surface_add_listener(state.xdg.surface, &g_XDGSurfaceListener, &window);

    state.xdg.toplevel = xdg_surface_get_toplevel(state.xdg.surface);
    if (!state.xdg.toplevel)
    {
        Log::Errorf("Wayland: Failed to create xdg-toplevel for window\n");
        return false;
    }

    xdg_toplevel_add_listener(state.xdg.toplevel, &g_XDGTopLevelListener, &window);

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

    if (LinuxWaylandState::GetDecorationManager())
    {
        state.xdg.decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(LinuxWaylandState::GetDecorationManager(), state.xdg.toplevel);
        zxdg_toplevel_decoration_v1_add_listener(state.xdg.decoration, &g_XDGDecorationListener, &window);

        // TODO: Let the user choose the mode?
        std::uint32_t mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
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
    wl_display_roundtrip(LinuxWaylandState::GetDisplay());
    return true;
}

static bool CreateWaylandWindowShellObjects(LinuxWindowWayland& window)
{
    if (LinuxWaylandState::GetLibdecor().context)
    {
        if (CreateLibdecorFrame(window))
            return true;
    }
    return CreateXdgShellObjects(window);
}

static void DestroyWaylandWindowShellObjects(LinuxWindowWayland& window)
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

static void SetWaylandWindowDecorated(LinuxWindowWayland& window, bool decorated)
{
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
    LinuxWaylandState::AddWindow(this);
    SetDesc(desc);
    OpenNativeWindow();
}

void LinuxWindowWayland::OpenNativeWindow()
{
    state_.wl.surface = wl_compositor_create_surface(LinuxWaylandState::GetCompositor());
    if (!state_.wl.surface)
        LLGL_TRAP("Failed to get Wayland surface");

    wl_proxy_set_tag(reinterpret_cast<wl_proxy*>(state_.wl.surface), LinuxWaylandState::GetTag());
    wl_surface_add_listener(state_.wl.surface, &g_WaylandSurfaceListener, nullptr);
    wl_surface_set_user_data(state_.wl.surface, this);

    wl_surface_commit(state_.wl.surface);
    wl_display_roundtrip(LinuxWaylandState::GetDisplay());

    WaylandResizeWindow(*this, desc_.size.width, desc_.size.height);

    WaylandSetContentAreaOpaque(state_);

    if (state_.visible)
    {
        CreateWaylandWindowShellObjects(*this);
    }
}

void LinuxWindowWayland::ProcessEventsInternal()
{
    if (state_.shouldClose)
        PostQuit();
}

void LinuxWindowWayland::ProcessMotionEvent(int xpos, int ypos)
{
    const Offset2D mousePos{ xpos, ypos };
    PostLocalMotion(mousePos);

    const Offset2D mouseMotion
    {
        mousePos.x - state_.prevMousePos.x,
        mousePos.y - state_.prevMousePos.y
    };
    PostGlobalMotion(mouseMotion);

    state_.prevMousePos = mousePos;
}

bool LinuxWindowWayland::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(NativeHandle))
    {
        auto* handle = static_cast<NativeHandle*>(nativeHandle);
        handle->type = NativeType::Wayland;
        handle->wayland.display = LinuxWaylandState::GetDisplay();
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
        if (visible && !state_.libdecor.frame && !state_.xdg.toplevel)
        {
            visible = CreateWaylandWindowShellObjects(*this);
        }
        else
        {
            DestroyWaylandWindowShellObjects(*this);
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
    if (!WaylandResizeWindow(*this, size.width, size.height))
        return;

    if (state_.libdecor.frame)
    {
        libdecor_state* frameState = libdecor_state_new(size.width, size.height);
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
    SetWaylandWindowDecorated(*this, state_.decorated);
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
    LinuxWaylandState::RemoveWindow(this);

    DestroyWaylandWindowShellObjects(*this);

    if (state_.fallback.buffer)
        wl_buffer_destroy(state_.fallback.buffer);

    if (state_.wl.surface)
        wl_surface_destroy(state_.wl.surface);
}


} // /namespace LLGL



// ================================================================================
