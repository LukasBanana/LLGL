/*
 * LinuxDisplay.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_LINUX_DISPLAY_H
#define LLGL_LINUX_DISPLAY_H


#include <LLGL/Display.h>
#include <memory>
#include <X11/Xlib.h>


namespace LLGL
{


// Helper class to handle a shared X11 display instance
class LinuxSharedX11Display
{

    public:

        LinuxSharedX11Display();
        ~LinuxSharedX11Display();

        // Returns the native X11 display.
        inline ::Display* GetNative() const
        {
            return native_;
        }

    private:

        ::Display* native_ = nullptr;

};

class LinuxDisplay : public Display
{

    public:

        LinuxDisplay(const std::shared_ptr<LinuxSharedX11Display>& sharedX11Display, int screenIndex);

        bool IsPrimary() const override;

        std::wstring GetDeviceName() const override;

        Offset2D GetOffset() const override;

        bool ResetDisplayMode() override;
        bool SetDisplayMode(const DisplayModeDescriptor& displayModeDesc) override;
        DisplayModeDescriptor GetDisplayMode() const override;

        std::vector<DisplayModeDescriptor> GetSupportedDisplayModes() const override;

    private:

        // Returns the native X11 display.
        ::Display* GetNative() const;

    private:

        std::shared_ptr<LinuxSharedX11Display>  sharedX11Display_;
        int                                     screen_             = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
