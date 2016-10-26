/*
 * Window.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_WINDOW_H
#define LLGL_WINDOW_H


#include <string>
#include <memory>
#include <vector>
#include <LLGL/Export.h>
#include <LLGL/Key.h>
#include <LLGL/Types.h>


namespace LLGL
{


//! Window descriptor structure.
struct WindowDescriptor
{
    std::wstring    title;
    Point           position;                       //!< Window position (relative to the client area).
    Size            size;                           //!< Client area size.

    bool            visible             = false;
    bool            borderless          = false;
    bool            resizable           = false;
    bool            acceptDropFiles     = false;
    bool            preventForPowerSafe = false;
    bool            centered            = false;

    /**
    \brief Window context handle.
    \remarks If used, this must be casted from a platform specific structure:
    \code
    #include <LLGL/Platform/NativeHandle.h>
    //...
    LLGL::NativeContextHandle handle;
    //handle.parentWindow = ...
    windowDesc.windowContext = reinterpret_cast<const void*>(&handle);
    \endcode
    */
    const void*     windowContext       = nullptr;
};


// Interface for a Window class (also named Canvas)
class LLGL_EXPORT Window
{

    public:

        // Interface for all window listeners
        class LLGL_EXPORT EventListener
        {

            public:

                virtual ~EventListener();

            protected:

                friend class Window;

                virtual void OnProcessEvents(Window& sender);

                virtual void OnKeyDown(Window& sender, Key keyCode);
                virtual void OnKeyUp(Window& sender, Key keyCode);

                virtual void OnDoubleClick(Window& sender, Key keyCode);
                
                virtual void OnChar(Window& sender, wchar_t chr);

                virtual void OnWheelMotion(Window& sender, int motion);

                virtual void OnLocalMotion(Window& sender, const Point& position);
                virtual void OnGlobalMotion(Window& sender, const Point& motion);

                virtual void OnResize(Window& sender, const Size& clientAreaSize);

                //! Returns true if the specified window can quit, i.e. "ProcessEvents" returns false from now on.
                virtual bool OnQuit(Window& sender);

        };

        /* --- Common --- */

        virtual ~Window();

        static std::unique_ptr<Window> Create(const WindowDescriptor& desc);

        virtual void SetPosition(const Point& position) = 0;
        virtual Point GetPosition() const = 0;

        virtual void SetSize(const Size& size, bool useClientArea = true) = 0;
        virtual Size GetSize(bool useClientArea = true) const = 0;

        virtual void SetTitle(const std::wstring& title) = 0;
        virtual std::wstring GetTitle() const = 0;

        virtual void Show(bool show = true) = 0;
        virtual bool IsShown() const = 0;

        //! Query a window descriptor, which describes the current state of this window.
        virtual WindowDescriptor QueryDesc() const = 0;

        //! Sets the new window descriptor.
        virtual void SetDesc(const WindowDescriptor& desc) = 0;

        /**
        \brief Recreates the internal window object. This may invalidate the native handle previously returned by "GetNativeHandle".
        \see GetNativeHandle
        */
        virtual void Recreate(const WindowDescriptor& desc) = 0;

        /**
        \brief Returns the native window handle.
        \remarks This must be casted to a platform specific structure:
        \code
        #include <LLGL/Platform/NativeHandle.h>
        //...
        LLGL::NativeHandle handle;
        window.GetNativeHandle(&handle);
        \endcode
        */
        virtual void GetNativeHandle(void* nativeHandle) const = 0;

        /**
        \brief Processes the events for this window (i.e. mouse movement, key presses etc.).
        \return Once the "PostQuit" function was called on this window object, this function returns false.
        This will happend, when the user clicks on the close button.
        */
        bool ProcessEvents();

        /* --- Event handling --- */

        void AddEventListener(const std::shared_ptr<EventListener>& eventListener);
        void RemoveEventListener(const EventListener* eventListener);

        void PostKeyDown(Key keyCode);
        void PostKeyUp(Key keyCode);

        void PostDoubleClick(Key keyCode);
        
        void PostChar(wchar_t chr);
        
        void PostWheelMotion(int motion);
        
        void PostLocalMotion(const Point& position);
        void PostGlobalMotion(const Point& motion);

        void PostResize(const Size& clientAreaSize);

        /**
        \brief Posts the 'OnQuit' event to all event listeners.
        \remarks If at least one event listener returns false within the "OnQuit" callback, the window will not quit.
        If all event listener return true within the "OnQuit" callback, "ProcessEvents" will returns false from now on.
        */
        void PostQuit();

    protected:

        virtual void ProcessSystemEvents() = 0;

    private:

        std::vector<std::shared_ptr<EventListener>> eventListeners_;

        bool quit_ = false;

};


} // /namespace LLGL


#endif



// ================================================================================
