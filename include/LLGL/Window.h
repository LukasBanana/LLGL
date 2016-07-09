/*
 * Window.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_WINDOW_H__
#define __LLGL_WINDOW_H__


#include <string>
#include <memory>
#include <LLGL/API.h>


namespace LLGL
{


struct WindowDesc
{
    std::wstring    title;
    int             x                   = 0; // X position of the client area
    int             y                   = 0; // Y position of the client area
    int             width               = 0; // Width of the client area
    int             height              = 0; // Height of the client area

    bool            visible             = false;
    bool            borderless          = false;
    bool            resizable           = false;
    bool            acceptDropFiles     = false;
    bool            preventForPowerSafe = false;
    bool            centered            = false;

    void*           parentWindow        = nullptr;
};


class LLGL_EXPORT Window
{

    public:

        virtual ~Window();

        static std::unique_ptr<Window> Create(const WindowDesc& desc);

        virtual void SetPosition(int x, int y) = 0;
        virtual void GetPosition(int& x, int& y) const = 0;

        virtual void SetSize(int width, int height, bool useClientArea = true) = 0;
        virtual void GetSize(int& width, int& height, bool useClientArea = true) const = 0;

        virtual void SetTitle(const std::wstring& title) = 0;
        virtual std::wstring GetTitle() const = 0;

        virtual void Show(bool show = true) = 0;
        virtual bool IsShown() const = 0;

        virtual const void* GetNativeHandle() const = 0;

        virtual bool ProcessEvents() = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
