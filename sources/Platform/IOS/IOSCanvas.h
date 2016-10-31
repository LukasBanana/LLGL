/*
 * IOSCanvas.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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

        void GetNativeHandle(void* nativeHandle) const override;

        void Recreate() override;

        Size GetContentSize() const override;

        void SetTitle(const std::wstring& title) override;
        std::wstring GetTitle() const override;

    private:
        
        void OnProcessEvents() override;

        CanvasDescriptor    desc_;

        UIView*             view_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
