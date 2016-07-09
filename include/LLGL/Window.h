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
#include <vector>
#include <LLGL/API.h>
#include <LLGL/Key.h>


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


// Interface for a Window class (also named Canvas)
class LLGL_EXPORT Window
{

    public:

        // Interface for all window listeners
        class LLGL_EXPORT Listener
        {

            public:

                virtual ~Listener();

                virtual void OnKeyDown(Key keyCode);
                virtual void OnKeyUp(Key keyCode);
                
                virtual void OnChar(wchar_t chr);

                virtual void OnWheelMotion(int motion);

                virtual void OnLocalMotion(int x, int y);
                virtual void OnGlobalMotion(int dx, int dy);

        };

        /* --- Common --- */

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

        bool ProcessEvents();

        /* --- Event handling --- */

        void AddListener(const std::shared_ptr<Listener>& listener);
        void RemoveListener(const Listener* listener);

        void PostKeyDown(Key keyCode);
        void PostKeyUp(Key keyCode);
        
        void PostChar(wchar_t chr);
        
        void PostWheelMotion(int motion);
        
        void PostLocalMotion(int x, int y);
        void PostGlobalMotion(int dx, int dy);

        void PostQuit();

    protected:

        virtual void ProcessSystemEvents() = 0;

    private:

        std::vector<std::shared_ptr<Listener>> listeners_;

        bool quit_ = false;

};


} // /namespace LLGL


#endif



// ================================================================================
