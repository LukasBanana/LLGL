/*
 * LinuxDisplay.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifdef LLGL_LINUX_ENABLE_WAYLAND
    #include <wayland-client.h>
#endif

#include "LinuxDisplay.h"
#include "../../Core/CoreUtils.h"
#include <X11/extensions/Xrandr.h>
#include <dlfcn.h>

namespace LLGL
{


static std::vector<std::unique_ptr<LinuxX11Display>>   g_x11DisplayList;
static std::vector<std::unique_ptr<LinuxWaylandDisplay>>   g_waylandDisplayList;

static std::vector<Display*>                        g_displayRefList;
static Display*                                     g_primaryDisplay    = nullptr;


LinuxSharedX11DisplaySPtr LinuxSharedX11Display::GetShared()
{
    static LinuxSharedX11DisplaySPtr sharedX11Display = std::make_shared<LinuxSharedX11Display>();
    return sharedX11Display;
}

static bool UpdateX11DisplayList()
{
    LinuxSharedX11DisplaySPtr sharedX11Display = LinuxSharedX11Display::GetShared();

    const int screenCount = ScreenCount(sharedX11Display->GetNative());
    if (screenCount >= 0 && static_cast<std::size_t>(screenCount) != g_x11DisplayList.size())
    {
        g_x11DisplayList.resize(static_cast<std::size_t>(screenCount));
        for (int i = 0; i < screenCount; ++i)
        {
            g_x11DisplayList[i] = MakeUnique<LinuxX11Display>(sharedX11Display, i);
            if (i == DefaultScreen(sharedX11Display->GetNative()))
                g_primaryDisplay = g_x11DisplayList[i].get();
        }
        return true;
    }

    return false;
}

static bool UpdateWaylandDisplayList()
{
    // TODO
    return true;
}


/*
 * LinuxSharedX11Display class
 */

#if !LLGL_BUILD_STATIC_LIB
static void* g_retainedLibGL = nullptr;
#endif

LinuxSharedX11Display::LinuxSharedX11Display() :
    native_ { XOpenDisplay(nullptr) }
{
    LLGL_ASSERT(native_, "failed to open connection to X server");
}

LinuxSharedX11Display::~LinuxSharedX11Display()
{
    XCloseDisplay(native_);

    #if !LLGL_BUILD_STATIC_LIB
    /*
    If libGL.so was retained, release it now.
    This library must be unloaded *after* the connection to the X11 display is closed, because GL might have registerd shutdown callbacks.
    If we unload libGL.so too soon, XCloseDisplay() will crash with SIGSEGV on Linux.
    */
    if (g_retainedLibGL)
        dlclose(g_retainedLibGL);
    #endif
}

void LinuxSharedX11Display::RetainLibGL()
{
    #if !LLGL_BUILD_STATIC_LIB
    if (!g_retainedLibGL)
        g_retainedLibGL = dlopen("libGL.so", RTLD_LAZY);
    #endif
}

/*
 * LinuxSharedWaylandDisplay class
 */

LinuxSharedWaylandDisplay::LinuxSharedWaylandDisplay() :
    native_ { wl_display_connect(nullptr) }
{
    LLGL_ASSERT(native_, "failed to connect to Wayland compositor");
}

LinuxSharedWaylandDisplay::~LinuxSharedWaylandDisplay()
{
    wl_display_disconnect(native_);
}

/*
 * Display class
 */

std::size_t Display::Count()
{
    UpdateX11DisplayList();
    return g_x11DisplayList.size();
}

Display* const * Display::GetList()
{
    if (UpdateX11DisplayList() || g_displayRefList.empty())
    {
        /* Update reference list and append null terminator to array */
        g_displayRefList.clear();
        g_displayRefList.reserve(g_x11DisplayList.size() + 1);
        for (const auto& display : g_x11DisplayList)
            g_displayRefList.push_back(display.get());
        g_displayRefList.push_back(nullptr);
    }
    return g_displayRefList.data();
}

Display* Display::Get(std::size_t index)
{
    UpdateX11DisplayList();
    return (index < g_x11DisplayList.size() ? g_x11DisplayList[index].get() : nullptr);
}

Display* Display::GetPrimary()
{
    UpdateX11DisplayList();
    return g_primaryDisplay;
}

bool Display::ShowCursor(bool show)
{
    //TODO
    return false;
}

bool Display::IsCursorShown()
{
    //TODO
    return true;
}

bool Display::SetCursorPosition(const Offset2D& position)
{
    LinuxSharedX11DisplaySPtr sharedX11Display = LinuxSharedX11Display::GetShared();
    ::Display* dpy = sharedX11Display->GetNative();
    ::Window rootWnd = DefaultRootWindow(dpy);
    XWarpPointer(
        /*display*/     dpy,
        /*src_w*/       None,
        /*dest_w*/      rootWnd,
        /*src_x*/       0,
        /*src_y*/       0,
        /*src_width*/   0,
        /*src_height*/  0,
        /*dest_x*/      position.x,
        /*dest_y*/      position.y
    );
    XFlush(dpy);
    return true;
}

Offset2D Display::GetCursorPosition()
{
    LinuxSharedX11DisplaySPtr sharedX11Display = LinuxSharedX11Display::GetShared();
    ::Display* dpy = sharedX11Display->GetNative();
    ::Window rootWnd = DefaultRootWindow(dpy);
    ::Window rootWndReturn, childWndReturn;
    unsigned int mask;
    Offset2D rootPosition = { 0, 0 };
    Offset2D childPosition = { 0, 0 };
    XQueryPointer(
        /*display*/         dpy,
        /*w*/               rootWnd,
        /*root_return*/     &rootWndReturn,
        /*child_return*/    &childWndReturn,
        /*root_x_return*/   &rootPosition.x,
        /*root_y_return*/   &rootPosition.y,
        /*win_x_return*/    &childPosition.x,
        /*win_y_return*/    &childPosition.y,
        /*mask_return*/     &mask
    );
    return rootPosition;
}


/*
 * LinuxDisplay class
 */

LinuxX11Display::LinuxX11Display(const std::shared_ptr<LinuxSharedX11Display>& sharedX11Display, int screenIndex) :
    sharedX11Display_ { sharedX11Display },
    screen_           { screenIndex      }
{
}

bool LinuxX11Display::IsPrimary() const
{
    return (screen_ == DefaultScreen(GetNative()));
}

UTF8String LinuxX11Display::GetDeviceName() const
{
    return UTF8String{ DisplayString(GetNative()) };
}

Offset2D LinuxX11Display::GetOffset() const
{
    /* Get display offset from position of root window */
    XWindowAttributes attribs = {};
    XGetWindowAttributes(GetNative(), RootWindow(GetNative(), screen_), &attribs);
    return Offset2D
    {
        attribs.x, attribs.y
    };
}

float LinuxX11Display::GetScale() const
{
    return 1.0f; // dummy
}

bool LinuxX11Display::ResetDisplayMode()
{
    //TODO
    return false;
}

bool LinuxX11Display::SetDisplayMode(const DisplayMode& displayMode)
{
    ::Display* dpy = GetNative();
    ::Window rootWnd = RootWindow(dpy, screen_);

    /* Get all screen sizes from X11 extension Xrandr */
    int numSizes = 0;
    XRRScreenSize* scrSizes = XRRSizes(GetNative(), screen_, &numSizes);

    for (int i = 0; i < numSizes; ++i)
    {
        /* Check if specified display mode resolution matches this screen configuration */
        if (displayMode.resolution.width  == static_cast<std::uint32_t>(scrSizes[i].width) &&
            displayMode.resolution.height == static_cast<std::uint32_t>(scrSizes[i].height))
        {
            if (XRRScreenConfiguration* scrCfg = XRRGetScreenInfo(dpy, rootWnd))
            {
                Status status = XRRSetScreenConfig(dpy, scrCfg, rootWnd, i, RR_Rotate_0, 0);
                XRRFreeScreenConfigInfo(scrCfg);
                return (status != 0);
            }
        }
    }

    return false;
}

DisplayMode LinuxX11Display::GetDisplayMode() const
{
    DisplayMode displayMode;

    ::Display* dpy = GetNative();
    if (::Screen* scr = ScreenOfDisplay(dpy, screen_))
    {
        ::Window rootWnd = RootWindow(dpy, screen_);

        /* Get screen resolution from X11 screen */
        displayMode.resolution.width    = static_cast<std::uint32_t>(scr->width);
        displayMode.resolution.height   = static_cast<std::uint32_t>(scr->height);

        /* Get refresh reate from X11 extension Xrandr */
        if (XRRScreenConfiguration* scrCfg = XRRGetScreenInfo(dpy, rootWnd))
        {
            displayMode.refreshRate = static_cast<std::uint32_t>(XRRConfigCurrentRate(scrCfg));
            XRRFreeScreenConfigInfo(scrCfg);
        }
    }

    return displayMode;
}

std::vector<DisplayMode> LinuxX11Display::GetSupportedDisplayModes() const
{
    std::vector<DisplayMode> displayModes;

    DisplayMode displayMode;

    /* Get all screen sizes from X11 extension Xrandr */
    int numSizes = 0;
    XRRScreenSize* scrSizes = XRRSizes(GetNative(), screen_, &numSizes);

    for (int i = 0; i < numSizes; ++i)
    {
        /* Initialize resolution */
        displayMode.resolution.width    = static_cast<std::uint32_t>(scrSizes[i].width);
        displayMode.resolution.height   = static_cast<std::uint32_t>(scrSizes[i].height);

        /* Add one display mode for each rate */
        int numRates = 0;
        short* rates = XRRRates(GetNative(), screen_, i, &numRates);

        for (int j = 0; j < numRates; ++j)
        {
            displayMode.refreshRate = static_cast<std::uint32_t>(rates[j]);
            displayModes.push_back(displayMode);
        }
    }

    /* Sort final display mode list and remove duplciate entries */
    FinalizeDisplayModes(displayModes);

    return displayModes;
}


/*
 * ======= Private: =======
 */

::Display* LinuxX11Display::GetNative() const
{
    return sharedX11Display_->GetNative();
}


} // /namespace LLGL



// ================================================================================
