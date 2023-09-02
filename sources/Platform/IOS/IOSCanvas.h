/*
 * IOSCanvas.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_IOS_CANVAS_H
#define LLGL_IOS_CANVAS_H


#include <UIKit/UIKit.h>
#include <LLGL/Canvas.h>


namespace LLGL
{


class IOSCanvas : public Canvas
{

    public:

        IOSCanvas(const CanvasDescriptor& desc);
        ~IOSCanvas();

        bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) override;

        Extent2D GetContentSize() const override;

        void SetTitle(const UTF8String& title) override;
        UTF8String GetTitle() const override;

        void ResetPixelFormat() override;

    public:

        // Returns the native UIWindow.
        inline UIWindow* GetUIWindow() const
        {
            return wnd_;
        }

    private:

        void OnProcessEvents() override;

    private:

        CanvasDescriptor    desc_;

        UIViewController*   viewController_ = nullptr;
        UIWindow*           wnd_            = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
