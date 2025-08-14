/*
 * LinuxDisplayWayland.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#if LLGL_LINUX_ENABLE_WAYLAND

#include "LinuxDisplayWayland.h"
#include <X11/extensions/Xrandr.h>
#include <dlfcn.h>

namespace LLGL
{

LinuxDisplayWayland::LinuxDisplayWayland(const WaylandDisplayData& data) :
    data_(data)
{
}

bool LinuxDisplayWayland::IsPrimary() const
{
    // TODO
    return true;
}

UTF8String LinuxDisplayWayland::GetDeviceName() const {
    return data_.name;
}

Offset2D LinuxDisplayWayland::GetOffset() const {
    return Offset2D(data_.x, data_.y);
}

float LinuxDisplayWayland::GetScale() const {
    return data_.scale;
}

bool LinuxDisplayWayland::ResetDisplayMode() {
    // TODO
    return true;
}

bool LinuxDisplayWayland::SetDisplayMode(const DisplayMode& displayMode) {
    // TODO
    return true;
}

DisplayMode LinuxDisplayWayland::GetDisplayMode() const {
    return data_.displayModes[data_.currentdisplayMode];
}

std::vector<DisplayMode> LinuxDisplayWayland::GetSupportedDisplayModes() const {
    return data_.displayModes;
}

bool LinuxDisplayWayland::SetCursorPositionInternal(const Offset2D &position) {
    // TODO
    return true;
}

Offset2D LinuxDisplayWayland::GetCursorPositionInternal() {
    // TODO
    return Offset2D(0, 0);
}

struct wl_output* LinuxDisplayWayland::GetNative() const {
    return data_.output;
}

} // /namespace LLGL

#endif

// ================================================================================
