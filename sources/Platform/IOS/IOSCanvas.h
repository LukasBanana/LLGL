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

        #include <LLGL/Backend/Canvas.inl>

    public:

        IOSCanvas(const CanvasDescriptor& desc);
        ~IOSCanvas();

    public:

        // Returns the native UIWindow.
        inline UIWindow* GetUIWindow() const
        {
            return wnd_;
        }

    private:

        CanvasDescriptor    desc_;

        UIViewController*   viewController_ = nullptr;
        UIWindow*           wnd_            = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
