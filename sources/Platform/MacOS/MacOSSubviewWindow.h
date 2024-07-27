/*
 * MacOSSubviewWindow.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MACOS_SUBVIEW_WINDOW_H
#define LLGL_MACOS_SUBVIEW_WINDOW_H


#include <LLGL/Window.h>

#import <Cocoa/Cocoa.h>


namespace LLGL
{


class MacOSSubviewWindow : public Window
{

    public:

        MacOSSubviewWindow(const WindowDescriptor& desc);
        ~MacOSSubviewWindow();

        bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) override;

        Extent2D GetContentSize() const override;

        void SetPosition(const Offset2D& position) override;
        Offset2D GetPosition() const override;

        void SetSize(const Extent2D& size, bool useClientArea = true) override;
        Extent2D GetSize(bool useClientArea = true) const override;

        void SetTitle(const UTF8String& title) override;
        UTF8String GetTitle() const override;

        void Show(bool show = true) override;
        bool IsShown() const override;

        void SetDesc(const WindowDescriptor& desc) override;
        WindowDescriptor GetDesc() const override;

    public:

        // Returns the native NSView object.
        inline NSView* GetNSView() const
        {
            return view_;
        }

    private:

        NSView* CreateNSView(const WindowDescriptor& desc);

    private:

        NSView*     view_ = nullptr;
        UTF8String  title_;

};


} // /namespace LLGL


#endif



// ================================================================================
