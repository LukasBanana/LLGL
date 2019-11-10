/*
 * IOSCanvas.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

        bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const override;

        Extent2D GetContentSize() const override;

        void SetTitle(const std::wstring& title) override;
        std::wstring GetTitle() const override;

        void ResetPixelFormat() override;

    private:

        void OnProcessEvents() override;

        UIViewController* CreateViewController(const CanvasDescriptor& desc);
        UIView* CreateView(const CanvasDescriptor& desc);

    private:

        CanvasDescriptor    desc_;

        UIViewController*   viewController_ = nullptr;
        UIView*             view_           = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
