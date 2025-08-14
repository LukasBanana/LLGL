/*
 * LinuxDisplayX11.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_DISPLAY_X11_H
#define LLGL_LINUX_DISPLAY_X11_H

#include <memory>
#include <X11/Xlib.h>

#include "LinuxDisplay.h"

namespace LLGL
{


class LinuxSharedDisplayX11;

using LinuxSharedX11DisplaySPtr = std::shared_ptr<LinuxSharedDisplayX11>;

// Helper class to handle a shared X11 display instance
class LinuxSharedDisplayX11
{

    public:

        LinuxSharedDisplayX11();
        ~LinuxSharedDisplayX11();

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

class LinuxDisplayX11 : public LinuxDisplay
{

    public:

        LinuxDisplayX11(const std::shared_ptr<LinuxSharedDisplayX11>& sharedX11Display, int screenIndex);

        bool IsPrimary() const override;

        UTF8String GetDeviceName() const override;

        Offset2D GetOffset() const override;
        float GetScale() const override;

        bool ResetDisplayMode() override;
        bool SetDisplayMode(const DisplayMode& displayMode) override;
        DisplayMode GetDisplayMode() const override;

        std::vector<DisplayMode> GetSupportedDisplayModes() const override;

    private:

        virtual bool SetCursorPositionInternal(const Offset2D& position) override;
        virtual Offset2D GetCursorPositionInternal() override;

        // Returns the native X11 display.
        ::Display* GetNative() const;

    private:

        std::shared_ptr<LinuxSharedDisplayX11>  sharedX11Display_;
        int                                     screen_             = 0;

};

} // /namespace LLGL


#endif



// ================================================================================
