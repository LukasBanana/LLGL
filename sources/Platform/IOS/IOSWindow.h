/*
 * IOSWindow.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_IOS_WINDOW_H
#define LLGL_IOS_WINDOW_H


#include <UIKit/UIKit.h>
#include <LLGL/Window.h>


namespace LLGL
{


class IOSWindow : public Window
{

    public:

        IOSWindow(const WindowDescriptor& desc);
        ~IOSWindow();

        void SetPosition(const Point& position) override;
        Point GetPosition() const override;

        void SetSize(const Size& size, bool useClientArea = true) override;
        Size GetSize(bool useClientArea = true) const override;

        void SetTitle(const std::wstring& title) override;
        std::wstring GetTitle() const override;

        void Show(bool show = true) override;
        bool IsShown() const override;

        WindowDescriptor QueryDesc() const override;

        void SetDesc(const WindowDescriptor& desc) override;

        void Recreate(const WindowDescriptor& desc) override;

        void GetNativeHandle(void* nativeHandle) const override;

    private:
        
        void OnProcessEvents() override;

        WindowDescriptor    desc_;

        UIView*             view_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
