/*
 * LinuxWindow.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Display.h>
#include "LinuxWindow.h"
#include "MapKey.h"
#include "../../Core/CoreUtils.h"
#include "../../Core/Exception.h"
#include <X11/Xresource.h>

#ifdef LLGL_LINUX_ENABLE_WAYLAND
    #include <wayland-client.h>
#endif


namespace LLGL
{


/*
 * LinuxX11Context class
 */

// Wrapper for XContext singleton.
class LinuxX11Context
{

    public:
        
        static void Save(::Display* display, XID id, void* userData);
        static void* Find(::Display* display, XID id);
        static void Remove(::Display* display, XID id);

    private:

        LinuxX11Context();

        static LinuxX11Context& Get();

    private:

        ::XContext ctx_;

};

LinuxX11Context::LinuxX11Context() :
    ctx_ { XUniqueContext() }
{
}

void LinuxX11Context::Save(::Display* display, XID id, void* userData)
{
    XSaveContext(display, id, LinuxX11Context::Get().ctx_, static_cast<XPointer>(userData));
}

void* LinuxX11Context::Find(::Display* display, XID id)
{
    XPointer userData = nullptr;
    XFindContext(display, id, LinuxX11Context::Get().ctx_, &userData);
    return reinterpret_cast<void*>(userData);
}

void LinuxX11Context::Remove(::Display* display, XID id)
{
    XDeleteContext(display, id, LinuxX11Context::Get().ctx_);
}

LinuxX11Context& LinuxX11Context::Get()
{
    static LinuxX11Context instance;
    return instance;
}


/*
 * Surface class
 */

bool Surface::ProcessEvents()
{
    ::Display* display = LinuxSharedX11Display::GetShared()->GetNative();

    XEvent event;

    XPending(display);

    while (XQLength(display))
    {
        XNextEvent(display, &event);
        if (void* userData = LinuxX11Context::Find(display, event.xany.window))
        {
            LinuxX11Window* wnd = static_cast<LinuxX11Window*>(userData);
            wnd->ProcessEvent(event);
        }
    }

    XFlush(display);

    return true;
}


/*
 * Window class
 */

static Offset2D GetScreenCenteredPosition(const Extent2D& size)
{
    if (auto display = Display::GetPrimary())
    {
        const auto resolution = display->GetDisplayMode().resolution;
        return
        {
            static_cast<int>((resolution.width  - size.width )/2),
            static_cast<int>((resolution.height - size.height)/2),
        };
    }
    return {};
}

std::unique_ptr<Window> Window::Create(const WindowDescriptor& desc)
{
    if (desc.wayland) {
        return MakeUnique<LinuxWaylandWindow>(desc);
    } else {
        return MakeUnique<LinuxX11Window>(desc);
    }
}


/*
 * LinuxX11Window class
 */

LinuxX11Window::LinuxX11Window(const WindowDescriptor& desc) :
    desc_ { desc }
{
    Open();
    LinuxX11Context::Save(display_, wnd_, this);
}

LinuxX11Window::~LinuxX11Window()
{
    LinuxX11Context::Remove(display_, wnd_);
    XDestroyWindow(display_, wnd_);
}

bool LinuxX11Window::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(NativeHandle))
    {
        auto* handle = static_cast<NativeHandle*>(nativeHandle);
        handle->type        = NativeType::X11;
        handle->x11.display = display_;
        handle->x11.window  = wnd_;
        handle->x11.visual  = visual_;
        return true;
    }
    return false;
}

Extent2D LinuxX11Window::GetContentSize() const
{
    /* Return the size of the client area */
    return GetSize(true);
}

void LinuxX11Window::SetPosition(const Offset2D& position)
{
    /* Move window and store new position */
    XMoveWindow(display_, wnd_, position.x, position.y);
    desc_.position = position;
}

Offset2D LinuxX11Window::GetPosition() const
{
    XWindowAttributes attribs;
    XGetWindowAttributes(display_, wnd_, &attribs);
    return { attribs.x, attribs.y };
}

void LinuxX11Window::SetSize(const Extent2D& size, bool useClientArea)
{
    XResizeWindow(display_, wnd_, size.width, size.height);
}

Extent2D LinuxX11Window::GetSize(bool useClientArea) const
{
    XWindowAttributes attribs;
    XGetWindowAttributes(display_, wnd_, &attribs);
    return Extent2D
    {
        static_cast<std::uint32_t>(attribs.width),
        static_cast<std::uint32_t>(attribs.height)
    };
}

void LinuxX11Window::SetTitle(const UTF8String& title)
{
    XStoreName(display_, wnd_, title.c_str());
}

UTF8String LinuxX11Window::GetTitle() const
{
    char* title = nullptr;
    XFetchName(display_, wnd_, &title);
    return title;
}

void LinuxX11Window::Show(bool show)
{
    if (show)
    {
        /* Map window and reset window position */
        XMapWindow(display_, wnd_);
        XMoveWindow(display_, wnd_, desc_.position.x, desc_.position.y);
    }
    else
        XUnmapWindow(display_, wnd_);

    if ((desc_.flags & WindowFlags::Borderless) != 0)
        XSetInputFocus(display_, (show ? wnd_ : None), RevertToParent, CurrentTime);
}

bool LinuxX11Window::IsShown() const
{
    XWindowAttributes attr;
    if (XGetWindowAttributes(display_, wnd_, &attr) == 0)
    {
        return false;
    }
    return attr.map_state == IsViewable;
}

void LinuxX11Window::SetDesc(const WindowDescriptor& desc)
{
    //todo...
}

WindowDescriptor LinuxX11Window::GetDesc() const
{
    return desc_; //todo...
}

void LinuxX11Window::ProcessEvent(XEvent& event)
{
    switch (event.type)
    {
        case KeyPress:
            ProcessKeyEvent(event.xkey, true);
            break;

        case KeyRelease:
            ProcessKeyEvent(event.xkey, false);
            break;

        case ButtonPress:
            ProcessMouseKeyEvent(event.xbutton, true);
            break;

        case ButtonRelease:
            ProcessMouseKeyEvent(event.xbutton, false);
            break;

        case Expose:
            ProcessExposeEvent();
            break;

        case MotionNotify:
            ProcessMotionEvent(event.xmotion);
            break;

        case DestroyNotify:
            PostQuit();
            break;

        case ClientMessage:
            ProcessClientMessage(event.xclient);
            break;
    }
}


/*
 * ======= Private: =======
 */

void LinuxX11Window::Open()
{
    /* Get native context handle */
    const NativeHandle* nativeHandle = static_cast<const NativeHandle*>(desc_.windowContext);
    if (nativeHandle != nullptr)
    {
        /* Get X11 display from context handle */
        LLGL_ASSERT(desc_.windowContextSize == sizeof(NativeHandle));
        LLGL_ASSERT(nativeHandle->type == NativeHandleType::X11, "Window native handle type must be X11");
        display_    = nativeHandle->x11.display;
        visual_     = nativeHandle->x11.visual;
    }
    else
    {
        /* Get shared X11 display */
        sharedX11Display_   = LinuxSharedX11Display::GetShared();
        display_            = sharedX11Display_->GetNative();
        visual_             = nullptr;
    }

    if (!display_)
        LLGL_TRAP("failed to open X11 display");

    /* Setup common parameters for window creation */
    ::Window    rootWnd     = (nativeHandle != nullptr ? nativeHandle->x11.window : DefaultRootWindow(display_));
    int         screen      = (nativeHandle != nullptr ? nativeHandle->x11.screen : DefaultScreen(display_));
    ::Visual*   visual      = (nativeHandle != nullptr ? nativeHandle->x11.visual->visual : DefaultVisual(display_, screen));
    int         depth       = (nativeHandle != nullptr ? nativeHandle->x11.visual->depth : DefaultDepth(display_, screen));
    int         borderSize  = 0;

    /* Setup window attributes */
    XSetWindowAttributes attribs;
    attribs.background_pixel    = WhitePixel(display_, screen);
    attribs.border_pixel        = 0;
    attribs.event_mask          = (ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

    unsigned long valueMask     = CWEventMask | CWBorderPixel;//(CWColormap | CWEventMask | CWOverrideRedirect)

    if (nativeHandle)
    {
        valueMask |= CWColormap;
        attribs.colormap = nativeHandle->x11.colorMap;
    }
    else
        valueMask |= CWBackPixel;

    const bool isBorderless = ((desc_.flags & WindowFlags::Borderless) != 0);
    if (isBorderless) //TODO: -> input no longer works
    {
        valueMask |= CWOverrideRedirect;
        attribs.override_redirect = true;
    }

    /* Get final window position */
    if ((desc_.flags & WindowFlags::Centered) != 0)
        desc_.position = GetScreenCenteredPosition(desc_.size);

    /* Create X11 window */
    wnd_ = XCreateWindow(
        display_,
        rootWnd,
        desc_.position.x,
        desc_.position.y,
        desc_.size.width,
        desc_.size.height,
        borderSize,
        depth,
        InputOutput,
        visual,
        valueMask,
        (&attribs)
    );

    /* Set title and show window (if enabled) */
    SetTitle(desc_.title);

    /* Show window */
    if ((desc_.flags & WindowFlags::Visible) != 0)
        Show();

    /* Prepare borderless window */
    if (isBorderless)
    {
        XGrabKeyboard(display_, wnd_, True, GrabModeAsync, GrabModeAsync, CurrentTime);
        XGrabPointer(display_, wnd_, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, wnd_, None, CurrentTime);
    }

    /* Enable WM_DELETE_WINDOW protocol */
    closeWndAtom_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display_, wnd_, &closeWndAtom_, 1);
}

void LinuxX11Window::ProcessKeyEvent(XKeyEvent& event, bool down)
{
    auto key = MapKey(event);
    if (down)
        PostKeyDown(key);
    else
        PostKeyUp(key);
}

void LinuxX11Window::ProcessMouseKeyEvent(XButtonEvent& event, bool down)
{
    switch (event.button)
    {
        case Button1:
            PostMouseKeyEvent(Key::LButton, down);
            break;
        case Button2:
            PostMouseKeyEvent(Key::MButton, down);
            break;
        case Button3:
            PostMouseKeyEvent(Key::RButton, down);
            break;
        case Button4:
            PostWheelMotion(1);
            break;
        case Button5:
            PostWheelMotion(-1);
            break;
    }
}

void LinuxX11Window::ProcessExposeEvent()
{
    XWindowAttributes attribs;
    XGetWindowAttributes(display_, wnd_, &attribs);

    const Extent2D size
    {
        static_cast<std::uint32_t>(attribs.width),
        static_cast<std::uint32_t>(attribs.height)
    };

    PostResize(size);
}

void LinuxX11Window::ProcessClientMessage(XClientMessageEvent& event)
{
    Atom atom = static_cast<Atom>(event.data.l[0]);
    if (atom == closeWndAtom_)
        PostQuit();
}

void LinuxX11Window::ProcessMotionEvent(XMotionEvent& event)
{
    const Offset2D mousePos { event.x, event.y };
    PostLocalMotion(mousePos);
    PostGlobalMotion({ mousePos.x - prevMousePos_.x, mousePos.y - prevMousePos_.y });
    prevMousePos_ = mousePos;
}

void LinuxX11Window::PostMouseKeyEvent(Key key, bool down)
{
    if (down)
        PostKeyDown(key);
    else
        PostKeyUp(key);
}

/*
 * LinuxWaylandWindow class
 */

void SurfaceHandleEnter(void* userData, struct wl_surface* surface, struct wl_output* output)
{
    wayland_state* state = static_cast<wayland_state*>(userData);
}

void SurfaceHandleLeave(void* userData, struct wl_surface* surface, struct wl_output* output)
{
    wayland_state* state = static_cast<wayland_state*>(userData);
}

const static wl_surface_listener surfaceListener = {
    SurfaceHandleEnter,
    SurfaceHandleLeave
};

void PointerHandleEnter(void *userData,
                        struct wl_pointer *wl_pointer,
                        uint32_t serial,
                        struct wl_surface *surface,
                        wl_fixed_t surface_x,
                        wl_fixed_t surface_y)
{
    if (!surface)
        return;

    LinuxWaylandWindow* window = static_cast<LinuxWaylandWindow*>(userData);
    wayland_state& state = window->GetWaylandState();

    if (wl_proxy_get_tag(reinterpret_cast<struct wl_proxy*>(surface)) != &state.tag)
        return;

    state.serial = serial;
    state.pointerEnterSerial = serial;
    state.hovered = true;
}

void PointerHandleLeave(void *userData,
		                struct wl_pointer *wl_pointer,
		                uint32_t serial,
		                struct wl_surface *surface)
{
    if (!surface)
        return;

    LinuxWaylandWindow* window = static_cast<LinuxWaylandWindow*>(userData);
    wayland_state& state = window->GetWaylandState();

    if (wl_proxy_get_tag(reinterpret_cast<struct wl_proxy*>(surface)) != &state.tag)
        return;

    state.serial = serial;
    state.hovered = false;
}

void PointerHandleMotion(void *userData,
		                 struct wl_pointer *wl_pointer,
		                 uint32_t time,
		                 wl_fixed_t surface_x,
		                 wl_fixed_t surface_y)
{
    LinuxWaylandWindow* window = static_cast<LinuxWaylandWindow*>(userData);
    wayland_state& state = window->GetWaylandState();

    if (state.hovered) {
        const int xpos = wl_fixed_to_int(surface_x);
        const int ypos = wl_fixed_to_int(surface_y);

        window->ProcessMotionEvent(xpos, ypos);
    }
}

void PointerHandleButton(void *userData,
		                 struct wl_pointer *wl_pointer,
		                 uint32_t serial,
		                 uint32_t time,
		                 uint32_t button,
		                 uint32_t state)
{
    LinuxWaylandWindow* window = static_cast<LinuxWaylandWindow*>(userData);
    wayland_state& wl = window->GetWaylandState();

    if (!wl.hovered)
        return;

    wl.serial = serial;

    window->ProcessMouseKeyEvent(button, state == WL_POINTER_BUTTON_STATE_PRESSED);
}

void PointerHandleAxis(void *userData,
		               struct wl_pointer *wl_pointer,
		               uint32_t time,
		               uint32_t axis,
		               wl_fixed_t value)
{
    LinuxWaylandWindow* window = static_cast<LinuxWaylandWindow*>(userData);
    wayland_state& wl = window->GetWaylandState();

    if (!wl.hovered)
        return;

    // TODO
}

const static wl_pointer_listener pointerListener = {
    PointerHandleEnter,
    PointerHandleLeave,
    PointerHandleMotion,
    PointerHandleButton,
    PointerHandleAxis
};

const static wl_keyboard_listener keyboardListener = {
    
};

void SeatHandleCapabilities(void* userData, struct wl_seat* seat, uint32_t caps)
{
    LinuxWaylandWindow* window = static_cast<LinuxWaylandWindow*>(userData);
    wayland_state& state = window->GetWaylandState();

    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !state.pointer)
    {
        state.pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(state.pointer, &pointerListener, window);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && state.pointer) {
        wl_pointer_destroy(state.pointer);
        state.pointer = nullptr;
    }

    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !state.keyboard)
    {
        state.keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(state.keyboard, &keyboardListener, window);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && state.keyboard) {
        wl_keyboard_destroy(state.keyboard);
        state.keyboard = nullptr;
    }
}

void SeatHandleName(void* userData, struct wl_seat* seat, const char* name)
{
}

const static struct wl_seat_listener seatListener = {
    SeatHandleCapabilities,
    SeatHandleName
};

void RegistryHandleGlobal(void* userData,
                          struct wl_registry* registry,
                          uint32_t name,
                          const char* interface,
                          uint32_t version)
{
    LinuxWaylandWindow* window = static_cast<LinuxWaylandWindow*>(userData);
    wayland_state& state = window->GetWaylandState();

    if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        state.compositor = static_cast<struct wl_compositor*>(
            wl_registry_bind(registry, name, &wl_compositor_interface, version)
        );
    }
    else if ((strcmp(interface, wl_seat_interface.name) == 0) && !state.seat) {
        state.seat = static_cast<struct wl_seat*>(
            wl_registry_bind(registry, name, &wl_seat_interface, version)
        );

        wl_seat_add_listener(state.seat, &seatListener, window);
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

LinuxWaylandWindow::LinuxWaylandWindow(const WindowDescriptor& desc) {
    Open();
}

void LinuxWaylandWindow::Open()
{
    struct wl_display* display = wl_display_connect(nullptr);
    if (!display)
        LLGL_TRAP("Failed to connect to the Wayland display");

    struct wl_registry* registry = wl_display_get_registry(display);

    wl_registry_add_listener(registry, &registryListener, this);

    wl_display_roundtrip(display);

    if (!wl_.compositor)
        LLGL_TRAP("Failed to get Wayland compositor");

    wl_.tag = "LLGL";

    wl_.surface = wl_compositor_create_surface(wl_.compositor);
    wl_proxy_set_tag(reinterpret_cast<struct wl_proxy*>(wl_.surface), &wl_.tag);
    wl_surface_add_listener(wl_.surface, &surfaceListener, &wl_);
}

void LinuxWaylandWindow::ProcessMotionEvent(int xpos, int ypos) {
    const Offset2D mousePos { xpos, ypos };
    PostLocalMotion(mousePos);
    PostGlobalMotion({ mousePos.x - prevMousePos_.x, mousePos.y - prevMousePos_.y });
    prevMousePos_ = mousePos;
}

bool LinuxWaylandWindow::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(NativeHandle))
    {
        auto* handle = static_cast<NativeHandle*>(nativeHandle);
        handle->type = NativeHandleType::Wayland;
        handle->wayland.display = wl_.display;
        handle->wayland.window  = wl_.surface;
        return true;
    }
    return false;
}

LinuxWaylandWindow::~LinuxWaylandWindow() {
    if (wl_.surface)
        wl_surface_destroy(wl_.surface);
    if (wl_.compositor)
        wl_compositor_destroy(wl_.compositor);
    if (wl_.pointer)
        wl_pointer_destroy(wl_.pointer);
    if (wl_.keyboard)
        wl_keyboard_destroy(wl_.keyboard);
    if (wl_.seat)
        wl_seat_destroy(wl_.seat);
    if (wl_.registry)
        wl_registry_destroy(wl_.registry);
    if (wl_.display)
    {
        wl_display_flush(wl_.display);
        wl_display_disconnect(wl_.display);
    }
}

} // /namespace LLGL



// ================================================================================
