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
#include <LLGL/Types.h>


namespace LLGL
{


struct WindowDesc
{
    std::wstring    title;
    Point           position;   // position (relative to the client area)
    Size            size;       // client area size

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

            protected:

                friend class Window;

                virtual void OnReset();

                virtual void OnKeyDown(Key keyCode);
                virtual void OnKeyUp(Key keyCode);
                
                virtual void OnChar(wchar_t chr);

                virtual void OnWheelMotion(int motion);

                virtual void OnLocalMotion(const Point& position);
                virtual void OnGlobalMotion(const Point& motion);

        };

        /* --- Common --- */

        virtual ~Window();

        static std::unique_ptr<Window> Create(const WindowDesc& desc);

        virtual void SetPosition(const Point& position) = 0;
        virtual Point GetPosition() const = 0;

        virtual void SetSize(const Size& size, bool useClientArea = true) = 0;
        virtual Size GetSize(bool useClientArea = true) const = 0;

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
        
        void PostLocalMotion(const Point& position);
        void PostGlobalMotion(const Point& motion);

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
