/*
 * Window.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_WINDOW_H
#define LLGL_WINDOW_H


#include "Surface.h"
#include "WindowFlags.h"
#include "Key.h"
#include <memory>
#include <vector>


namespace LLGL
{


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

        /**
        \brief Interface for all window event listeners.
        \remarks This is a design exception compared to most other interfaces in LLGL, because it does not inherit from the NonCopyable interface.
        This is because there is no hidden implementation, so copying an instance of this interface is allowed.
        \see Input
        */
        class LLGL_EXPORT EventListener
        {

            public:

                virtual ~EventListener() = default;

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
                virtual void OnLocalMotion(Window& sender, const Offset2D& position);

                //! Send when the global mouse position has changed. This is a raw input and independent of the screen resolution.
                virtual void OnGlobalMotion(Window& sender, const Offset2D& motion);

                //! Send when the window has been resized.
                virtual void OnResize(Window& sender, const Extent2D& clientAreaSize);

                //! Send when the window gets the keyboard focus.
                virtual void OnGetFocus(Window& sender);

                //! Send when the window loses the keyboard focus.
                virtual void OnLoseFocus(Window& sender);

                /**
                \brief Send when the window is about to be quit.
                \return True if the sender window can quit. In this case "ProcessEvents" returns false from now on.
                Otherwise the quit can be prevented. Returns true by default.
                \see Window::ProcessEvents
                */
                virtual bool OnQuit(Window& sender);

                /**
                \brief Send when the window received a timer event with the specified timer ID number.
                \note Only supported on: MS. Windows.
                */
                virtual void OnTimer(Window& sender, std::uint32_t timerID);

        };

        /* --- Common --- */

        /**
        \brief Creates a platform specific instance of the Window interface.
        \return Unique pointer to a new instance of the platform specific Window interface or
        null if the platform does not support windows (such as Android and iOS).
        \remarks For mobile platforms the interface Canvas can be used.
        \see Canvas
        */
        static std::unique_ptr<Window> Create(const WindowDescriptor& desc);

        //! Sets the window position relative to its parent.
        virtual void SetPosition(const Offset2D& position) = 0;

        //! Returns the window position relative to its parent.
        virtual Offset2D GetPosition() const = 0;

        //! Sets the either the overall window size or the client area size. By default the client area size is set.
        virtual void SetSize(const Extent2D& size, bool useClientArea = true) = 0;

        //! Returns either the overall window size or the client area size. By default the client area size is returned.
        virtual Extent2D GetSize(bool useClientArea = true) const = 0;

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

        //! Sets the mouse cursor position
        virtual void SetMousePosition(const Offset2D& Pos);
        
        //! Sets the new window behavior.
        virtual void SetBehavior(const WindowBehavior& behavior);

        //! Returns the window heavior.
        inline const WindowBehavior& GetBehavior() const
        {
            return behavior_;
        }

        //! Returns true if this window has the keyboard focus.
        virtual bool HasFocus() const;

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

        //! Adds the specified event listener to this window.
        void AddEventListener(const std::shared_ptr<EventListener>& eventListener);

        //! Removes the specified event listener from this window.
        void RemoveEventListener(const EventListener* eventListener);

        /**
        \brief Posts a 'KeyDown' event to all event listeners.
        \remarks This will be called automatically by the "ProcessEvents" function.
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
        void PostLocalMotion(const Offset2D& position);

        //! \see PostKeyDown
        void PostGlobalMotion(const Offset2D& motion);

        //! \see PostKeyDown
        void PostResize(const Extent2D& clientAreaSize);

        //! Posts a 'GetFocus' event to all event listeners.
        void PostGetFocus();

        //! Posts a 'LoseFocus' event to all event listeners.
        void PostLoseFocus();

        /**
        \brief Posts a 'Quit' event to all event listeners.
        \remarks If at least one event listener returns false within the "OnQuit" callback, the window will not quit.
        If all event listener return true within the "OnQuit" callback, "ProcessEvents" will returns false from now on.
        \see EventListener::OnQuit
        \see ProcessEvents
        */
        void PostQuit();

        /**
        \brief Posts a timer event with the specified timer ID number.
        \remarks This can be used to refresh the screen while the underlying window is currently being moved or resized by the user.
        \note Only supported on: MS. Windows.
        */
        void PostTimer(std::uint32_t timerID);

    protected:

        /**
        \brief Called inside the "ProcessEvents" function after all event listeners received the same event.
        \see ProcessEvents
        \see EventListener::OnProcessEvents
        */
        virtual void OnProcessEvents() = 0;

    private:

        std::vector<std::shared_ptr<EventListener>> eventListeners_;
        WindowBehavior                              behavior_;
        bool                                        quit_           = false;
        bool                                        hasFocus_       = false;

};


} // /namespace LLGL


#endif



// ================================================================================
