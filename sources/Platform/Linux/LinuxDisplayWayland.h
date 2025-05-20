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
#include <memory>
#include <wayland-client.h>


namespace LLGL
{


class LinuxSharedWaylandDisplay;
using LinuxSharedWaylandDisplaySPtr = std::shared_ptr<LinuxSharedWaylandDisplay>;


// Helper class to handle a shared Wayland display instance
class LinuxSharedWaylandDisplay
{

    public:

        LinuxSharedWaylandDisplay();
        ~LinuxSharedWaylandDisplay();

        // Returns a shared instance of the X11 display.
        static LinuxSharedWaylandDisplaySPtr GetShared();

        // Returns the native X11 display.
        inline struct wl_display* GetNative() const
        {
            return native_;
        }

    private:

        struct wl_display* native_ = nullptr;

};

class LinuxDisplayWayland : public Display
{

    public:

        LinuxDisplayWayland(const std::shared_ptr<LinuxSharedWaylandDisplay>& sharedWaylandDisplay, int screenIndex);

        bool IsPrimary() const override;

        UTF8String GetDeviceName() const override;

        Offset2D GetOffset() const override;
        float GetScale() const override;

        bool ResetDisplayMode() override;
        bool SetDisplayMode(const DisplayMode& displayMode) override;
        DisplayMode GetDisplayMode() const override;

        std::vector<DisplayMode> GetSupportedDisplayModes() const override;

        bool IsWayland() const override {
            return true;
        }

    private:

        // Returns the native Wayland display.
        struct wl_display* GetNative() const;

    private:

        std::shared_ptr<LinuxSharedWaylandDisplay>  sharedWaylandDisplay_;
        int                                         screen_             = 0;

};

} // /namespace LLGL

#endif // LLGL_LINUX_ENABLE_WAYLAND

#endif // LLGL_LINUX_DISPLAY_WAYLAND_H



// ================================================================================
