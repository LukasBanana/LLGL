/*
 * LinuxDisplayWayland.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_DISPLAY_WAYLAND_H
#define LLGL_LINUX_DISPLAY_WAYLAND_H

#if LLGL_LINUX_ENABLE_WAYLAND

#include <LLGL/Display.h>
#include <LLGL/Container/DynamicVector.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>

struct wl_output;

namespace LLGL
{


struct WaylandDisplayData {
    char name[128];

    std::vector<DisplayMode> displayModes;

    wl_output* output;

    uint32_t currentdisplayMode;

    uint32_t index;

    // Physical width in millimeters
    int widthMM;

    // Physical height in millimeters
    int heightMM;

    int x;
    int y;

    int scale;
};

class LinuxDisplayWayland : public Display
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
