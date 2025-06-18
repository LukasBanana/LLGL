/*
 * LinuxDisplayWayland.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#if LLGL_LINUX_ENABLE_WAYLAND

#include "LinuxDisplayWayland.h"
#include "../../Core/CoreUtils.h"
#include <X11/extensions/Xrandr.h>
#include <dlfcn.h>

namespace LLGL
{


static std::vector<std::unique_ptr<LinuxDisplayWayland>>   g_waylandDisplayList;

static std::vector<Display*>                        g_displayRefList;
static Display*                                     g_primaryDisplay    = nullptr;


static bool UpdateWaylandDisplayList()
{
    // TODO
    return true;
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

} // /namespace LLGL

#endif

// ================================================================================
