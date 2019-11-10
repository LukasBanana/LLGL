/*
 * Window.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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

        LLGL_DECLARE_INTERFACE( InterfaceID::Window );

    public:

        /**
        \brief Interface for all window event listeners.
        \remarks This is a design exception compared to most other interfaces in LLGL, because it does not inherit from the NonCopyable interface.
        This is because there is no hidden implementation, so copying an instance of this interface is allowed.
        \see Input
        */
        class LLGL_EXPORT EventListener : public Interface
        {

                LLGL_DECLARE_INTERFACE( InterfaceID::Window_EventListener );

            protected:

                friend class Window;

                /**
                \brief Send when the window events are about to be polled. The event listeners receive this event before the window itself.
                \see Window::OnProcessEvents
                */
                virtual void OnProcessEvents(Window& sender);

                /**
                \brief Send when the window is about to quit.
                \param[in] sender Specifies the sender of this event.
                \param[out] veto Specifies whether to cancel the quit event.
                If set to true, the call to \c PostQuit does not change the state \c sender, only the event listeners get informed.
                If no event listener sets this parameter to true, \c sender is set to the 'Quit' state and \c ProcessEvents returns false from then on.
                \see Window::ProcessEvents
                */
                virtual void OnQuit(Window& sender, bool& veto);

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

                //! Send when the window lost the keyboard focus.
                virtual void OnLostFocus(Window& sender);

                /**
                \brief Send when the window received a timer event with the specified timer ID number.
                \note Only supported on: MS. Windows.
                */
                virtual void OnTimer(Window& sender, std::uint32_t timerID);

        };

    public:

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

        //! Returns the window position relative to its parent (which can also be the display).
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
        \brief Returns true if this window is in the 'Quit' state.
        \see PostQuit
        \see ProcessEvents
        */
        virtual bool HasQuit() const;

        /**
        \brief Adapts the window for the specified video mode.
        \remarks This is a default implementation of the base class function and makes use of "GetDesc" and "SetDesc".
        \see GetDesc
        \see SetDesc
        */
        bool AdaptForVideoMode(VideoModeDescriptor& videoModeDesc) override;

        /**
        \brief Processes the events for this window (i.e. mouse movement, key presses etc.).
        \return True, as long as the window can process events.
        Once the \c PostQuit function has set this window to the 'Quit' state, this function returns false.
        This happens when the user clicks on the close button.
        \see PostQuit
        */
        bool ProcessEvents() override final;

        //! Searches the entire list of displays until a display is found where more than the half of this window's client area is visible.
        std::unique_ptr<Display> FindResidentDisplay() const override final;

        /* --- Event handling --- */

        //! Adds the specified event listener to this window.
        void AddEventListener(const std::shared_ptr<EventListener>& eventListener);

        //! Removes the specified event listener from this window.
        void RemoveEventListener(const EventListener* eventListener);

        /**
        \brief Posts a 'Quit' event to all event listeners if the window is not yet in the 'Quit' state.
        \remarks If one or more event listeners set the \c veto parameter to true in the \c OnQuit callback, the window will not quit.
        Otherwise, the \c ProcessEvents function will return false from then on.
        \see EventListener::OnQuit
        \see ProcessEvents
        \see HasQuit
        */
        void PostQuit();

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

        //! Posts a 'LostFocus' event to all event listeners.
        void PostLostFocus();

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
        bool                                        focus_          = false;

};


} // /namespace LLGL


#endif



// ================================================================================
