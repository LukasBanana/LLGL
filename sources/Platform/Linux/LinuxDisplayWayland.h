/*
 * LinuxDisplayWayland.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_DISPLAY_WAYLAND_H
#define LLGL_LINUX_DISPLAY_WAYLAND_H

#if LLGL_LINUX_ENABLE_WAYLAND

#include <LLGL/Container/DynamicVector.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>

#include "LinuxDisplay.h"

struct wl_output;

namespace LLGL
{


struct WaylandDisplayData {
    char deviceName[128] = {};

    std::vector<DisplayMode> displayModes;

    wl_output* output = nullptr;

    uint32_t currentdisplayMode = 0;

    uint32_t name = 0;

    // Physical width in millimeters
    int widthMM = 0;

    // Physical height in millimeters
    int heightMM = 0;

    int x = 0;
    int y = 0;

    int scale = 1;
};

class LinuxDisplayWayland : public LinuxDisplay
{

    public:

        LinuxDisplayWayland(const WaylandDisplayData& data);

        bool IsPrimary() const override;

        UTF8String GetDeviceName() const override;

        Offset2D GetOffset() const override;
        float GetScale() const override;

        bool ResetDisplayMode() override;
        bool SetDisplayMode(const DisplayMode& displayMode) override;
        DisplayMode GetDisplayMode() const override;

        std::vector<DisplayMode> GetSupportedDisplayModes() const override;

        WaylandDisplayData& GetData()
        {
            return data_;
        }

    private:

        bool SetCursorPositionInternal(const Offset2D &position) override;
        Offset2D GetCursorPositionInternal() override;

        // Returns the native Wayland display.
        wl_output* GetNative() const;

    private:
        WaylandDisplayData data_;

};

} // /namespace LLGL

#endif // LLGL_LINUX_ENABLE_WAYLAND

#endif // LLGL_LINUX_DISPLAY_WAYLAND_H



// ================================================================================
