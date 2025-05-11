/*
 * LinuxDisplay.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_DISPLAY_H
#define LLGL_LINUX_DISPLAY_H


#include <LLGL/Display.h>
#include <memory>
#include <X11/Xlib.h>


namespace LLGL
{


class LinuxSharedX11Display;
class LinuxSharedWaylandDisplay;

using LinuxSharedX11DisplaySPtr = std::shared_ptr<LinuxSharedX11Display>;
using LinuxSharedWaylandDisplaySPtr = std::shared_ptr<LinuxSharedWaylandDisplay>;

// Helper class to handle a shared X11 display instance
class LinuxSharedX11Display
{

    public:

        LinuxSharedX11Display();
        ~LinuxSharedX11Display();

        // Returns a shared instance of the X11 display.
        static LinuxSharedX11DisplaySPtr GetShared();

        // Notify to retain a reference to libGL.so for the shared X11 display connection.
        static void RetainLibGL();

        // Returns the native X11 display.
        inline ::Display* GetNative() const
        {
            return native_;
        }

    private:

        ::Display* native_ = nullptr;

};

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

class LinuxX11Display : public Display
{

    public:

        LinuxX11Display(const std::shared_ptr<LinuxSharedX11Display>& sharedX11Display, int screenIndex);

        bool IsPrimary() const override;

        UTF8String GetDeviceName() const override;

        Offset2D GetOffset() const override;
        float GetScale() const override;

        bool ResetDisplayMode() override;
        bool SetDisplayMode(const DisplayMode& displayMode) override;
        DisplayMode GetDisplayMode() const override;

        std::vector<DisplayMode> GetSupportedDisplayModes() const override;

    private:

        // Returns the native X11 display.
        ::Display* GetNative() const;

    private:

        std::shared_ptr<LinuxSharedX11Display>  sharedX11Display_;
        int                                     screen_             = 0;

};

class LinuxWaylandDisplay : public Display
{

    public:

        LinuxWaylandDisplay(const std::shared_ptr<LinuxSharedWaylandDisplay>& sharedWaylandDisplay, int screenIndex);

        bool IsPrimary() const override;

        UTF8String GetDeviceName() const override;

        Offset2D GetOffset() const override;
        float GetScale() const override;

        bool ResetDisplayMode() override;
        bool SetDisplayMode(const DisplayMode& displayMode) override;
        DisplayMode GetDisplayMode() const override;

        std::vector<DisplayMode> GetSupportedDisplayModes() const override;

    private:

        // Returns the native Wayland display.
        struct wl_display* GetNative() const;

    private:

        std::shared_ptr<LinuxSharedWaylandDisplay>  sharedWaylandDisplay_;
        int                                         screen_             = 0;

};

} // /namespace LLGL


#endif



// ================================================================================
