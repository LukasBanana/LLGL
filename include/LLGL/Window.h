/*
 * Window.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_WINDOW_H
#define LLGL_WINDOW_H


#include "Surface.h"
#include "Types.h"
#include "Key.h"
#include <string>
#include <memory>
#include <vector>


namespace LLGL
{


//! Value for an invalid window timer ID.
static const unsigned int invalidWindowTimerID = 0;

//! Window descriptor structure.
struct WindowDescriptor
{
    //! Window title as UTF16 string.
    std::wstring    title;

    //! Window position (relative to the client area).
    Point           position;

    //! Window size (this should be the client area size).
    Size            size;

    //! Specifies whether the window is visible at creation time.
    bool            visible             = false;

    //! Specifies whether the window is borderless. This is required for a fullscreen render context.
    bool            borderless          = false;

    //! Specifies whether the window can be resized.
    bool            resizable           = false;

    //! Specifies whether the window allows that files can be draged-and-droped onto the window.
    bool            acceptDropFiles     = false;

    /**
    \brief Specifies whether the 
    \note Only supported on: MS/Windows.
    */
    bool            preventForPowerSafe = false;

    //! Specifies whether the window is centered within the desktop screen.
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

/**
\brief Window behavior structure.
\see Window::SetBehavior
*/
struct WindowBehavior
{
    /**
    \brief Specifies whether to clear the content of the window when it is resized. By default false.
    \remarks This is used by Win32 to erase (WM_ERASEBKGND message) or keep the background on a window resize.
    If this is false, some kind of flickering during a window resize can be avoided.
    \note Only supported on: Win32.
    */
    bool            disableClearOnResize    = false;

    /**
    \brief Specifies an ID for a timer which will be activated when the window is moved or sized. By default invalidWindowTimerID.
    \remarks This is used by Win32 to set a timer during a window is moved or resized to make continous scene updates.
    Do not reset it during the 'OnTimer' event, otherwise a timer might be not be released correctly!
    \note Only supported on: Win32.
    \see Window::EventListener::OnTimer
    \see invalidWindowTimerID
    */
    unsigned int    moveAndResizeTimerID    = invalidWindowTimerID;
};


/**
\brief Window interface for desktop platforms.
\remarks This is the main interface for the windowing system in LLGL.
To implement a custom window (and use GLFW for instance) you have to
derive from this class and implement all pure virtual functions.
The couterpart is the Canvas interface for mobile platforms.
\see Canvas
*/
class LLGL_EXPORT Window : public Surface
{

    public:

        //! Interface for all window event listeners.
        class LLGL_EXPORT EventListener
        {

            public:

                virtual ~EventListener();

            protected:

                friend class Window;

                /**
                \brief Send when the window events are about to be polled. The event listeners receive this event before the window itself.
                \see Window::OnProcessEvents
                */
                virtual void OnProcessEvents(Window& sender);

                //! Send when a key (from keyboard or mouse) has been pushed.
                virtual void OnKeyDown(Window& sender, Key keyCode);

                //! Send when a key (from keyboard or mouse) has been released.
                virtual void OnKeyUp(Window& sender, Key keyCode);

                //! Send when a mouse button has been double clicked.
                virtual void OnDoubleClick(Window& sender, Key keyCode);
                
                //! Send when a character specific key has been typed on the sender window. This will repeat depending on the OS keyboard settings.
                virtual void OnChar(Window& sender, wchar_t chr);

                //! Send when the mouse wheel has been moved on the sender window.
                virtual void OnWheelMotion(Window& sender, int motion);

                //! Send when the mouse has been moved on the sender window.
                virtual void OnLocalMotion(Window& sender, const Point& position);

                //! Send when the global mouse position has changed. This is a raw input and independent of the screen resolution.
                virtual void OnGlobalMotion(Window& sender, const Point& motion);

                //! Send when the window has been resized.
                virtual void OnResize(Window& sender, const Size& clientAreaSize);

                /**
                \brief Send when the window is about to be quit.
                \return True if the sender window can quit. In this case "ProcessEvents" returns false from now on.
                Otherwise the quit can be prevented. Returns true by default.
                \see Window::ProcessEvents
                */
                virtual bool OnQuit(Window& sender);

                //! Send when the window received a timer event with the specified timer ID number.
                virtual void OnTimer(Window& sender, unsigned int timerID);

        };

        /* --- Common --- */

        virtual ~Window();

        /**
        \brief Creates a platform specific instance of the Window interface.
        \return Unique pointer to a new instance of the platform specific Window interface or
        null if the platform does not support windows (such as Android and iOS).
        \remarks For mobile platforms the interface Canvas can be used.
        \see Canvas
        */
        static std::unique_ptr<Window> Create(const WindowDescriptor& desc);

        //! Sets the window position relative to its parent.
        virtual void SetPosition(const Point& position) = 0;

        //! Returns the window position relative to its parent.
        virtual Point GetPosition() const = 0;

        //! Sets the either the overall window size or the client area size. By default the client area size is set.
        virtual void SetSize(const Size& size, bool useClientArea = true) = 0;

        //! Returns either the overall window size or the client area size. By default the client area size is returned.
        virtual Size GetSize(bool useClientArea = true) const = 0;

        //! Sets the window title as UTF16 string. If the OS does not support UTF16 window title, it will be converted to UTF8.
        virtual void SetTitle(const std::wstring& title) = 0;

        //! Returns the window title as UTF16 string.
        virtual std::wstring GetTitle() const = 0;

        //! Shows or hides the window.
        virtual void Show(bool show = true) = 0;

        //! Returns true if this window is visible.
        virtual bool IsShown() const = 0;

        /**
        \brief Sets the window attributes according to the specified window descriptor.
        \remarks This is used by the RenderContext interface when the video mode is about to change.
        \see RenderContext::SetVideoMode
        */
        virtual void SetDesc(const WindowDescriptor& desc) = 0;

        //! Queries a window descriptor, which describes the attributes of this window.
        virtual WindowDescriptor GetDesc() const = 0;

        //! Sets the new window behavior.
        virtual void SetBehavior(const WindowBehavior& behavior);

        //! Returns the window heavior.
        inline const WindowBehavior& GetBehavior() const
        {
            return behavior_;
        }

        /**
        \brief Adapts the window for the specified video mode.
        \remarks This is a default implementation of the base class function and makes use of "GetDesc" and "SetDesc".
        \see GetDesc
        \see SetDesc
        */
        bool AdaptForVideoMode(VideoModeDescriptor& videoModeDesc) override;

        /**
        \brief Processes the events for this window (i.e. mouse movement, key presses etc.).
        \return Once the "PostQuit" function was called on this window object, this function returns false.
        This will happend, when the user clicks on the close button.
        */
        bool ProcessEvents();

        /* --- Event handling --- */

        //! Adds a new event listener to this window.
        void AddEventListener(const std::shared_ptr<EventListener>& eventListener);

        //! Removes the specified event listener from this window.
        void RemoveEventListener(const EventListener* eventListener);

        /**
        \brief Posts a 'KeyDown' event to all event listeners.
        \remarks For a window created with "Window::Create", and events will be posted automatically by the "ProcessEvents" function.
        \see EventListener::OnKeyDown
        \see ProcessEvents
        */
        void PostKeyDown(Key keyCode);

        //! \see PostKeyDown
        void PostKeyUp(Key keyCode);

        //! \see PostKeyDown
        void PostDoubleClick(Key keyCode);
        
        //! \see PostKeyDown
        void PostChar(wchar_t chr);
        
        //! \see PostKeyDown
        void PostWheelMotion(int motion);
        
        //! \see PostKeyDown
        void PostLocalMotion(const Point& position);

        //! \see PostKeyDown
        void PostGlobalMotion(const Point& motion);

        //! \see PostKeyDown
        void PostResize(const Size& clientAreaSize);

        /**
        \brief Posts a 'Quit' event to all event listeners.
        \remarks If at least one event listener returns false within the "OnQuit" callback, the window will not quit.
        If all event listener return true within the "OnQuit" callback, "ProcessEvents" will returns false from now on.
        \see EventListener::OnQuit
        \see ProcessEvents
        */
        void PostQuit();

        //! \see PostKeyDown
        void PostTimer(unsigned int timerID);

    protected:

        /**
        \briefs Called inside the "ProcessEvents" function after all event listeners received the same event.
        \see ProcessEvents
        \see EventListener::OnProcessEvents
        */
        virtual void OnProcessEvents() = 0;

    private:

        std::vector<std::shared_ptr<EventListener>> eventListeners_;
        WindowBehavior                              behavior_;
        bool                                        quit_           = false;

};


} // /namespace LLGL


#endif



// ================================================================================
