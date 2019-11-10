/*
 * Canvas.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_CANVAS_H
#define LLGL_CANVAS_H


#include "CanvasFlags.h"
#include "Surface.h"
#include "Types.h"
#include "Key.h"
#include <memory>


namespace LLGL
{


/**
\brief Canvas interface for mobile platforms.
\remarks This is the main interface for the windowing system in LLGL on mobile platforms.
The couterpart is the Window interface for desktop platforms.
\see Window
*/
class LLGL_EXPORT Canvas : public Surface
{

        LLGL_DECLARE_INTERFACE( InterfaceID::Canvas );

    public:

        //! Interface for all canvas event listeners.
        class LLGL_EXPORT EventListener : public Interface
        {

                LLGL_DECLARE_INTERFACE( InterfaceID::Canvas_EventListener );

            protected:

                friend class Canvas;

                /**
                \brief Send when the canvas events are about to be polled. The event listeners receive this event before the canvas itself.
                \param[in] sender Specifies the sender of this event.
                \see Canvas::OnProcessEvents
                */
                virtual void OnProcessEvents(Canvas& sender);

                /**
                \brief Send when the canvas is about to quit.
                \param[in] sender Specifies the sender of this event.
                \param[out] veto Specifies whether to cancel the quit event.
                If set to true, the call to \c PostQuit does not change the state \c sender, only the event listeners get informed.
                If no event listener sets this parameter to true, \c sender is set to the 'Quit' state and \c ProcessEvents returns false from then on.
                \see Canvas::ProcessEvents
                */
                virtual void OnQuit(Canvas& sender, bool& veto);

        };

    public:

        /* --- Common --- */

        /**
        \brief Creates a platform specific instance of the Canvas interface.
        \return Unique pointer to a new instance of the platform specific Canvas interface or
        null if the platform does not support canvas (such as Windows, Linux, and macOS).
        \remarks For desktop platforms the interface Window can be used.
        \see Window
        */
        static std::unique_ptr<Canvas> Create(const CanvasDescriptor& desc);

        //! Sets the canvas title as UTF16 string. If the OS does not support UTF16 window title, it will be converted to UTF8.
        virtual void SetTitle(const std::wstring& title) = 0;

        //! Returns the canvas title as UTF16 string.
        virtual std::wstring GetTitle() const = 0;

        /**
        \brief Returns true if this canvas is in the 'Quit' state.
        \see PostQuit
        \see ProcessEvents
        */
        virtual bool HasQuit() const;

        //! This default implementation ignores the video mode descriptor completely and always return false.
        bool AdaptForVideoMode(VideoModeDescriptor& videoModeDesc) override;

        /**
        \brief Processes the events for this canvas (i.e. touch input, key presses etc.).
        \return True, as long as the window can process events.
        Once the \c PostQuit function has set this canvas to the 'Quit' state, this function returns false.
        \see PostQuit
        */
        bool ProcessEvents() override final;

        //! Always returns Display::InstantiatePrimary.
        std::unique_ptr<Display> FindResidentDisplay() const override final;

        /* --- Event handling --- */

        //! Adds a new event listener to this canvas.
        void AddEventListener(const std::shared_ptr<EventListener>& eventListener);

        //! Removes the specified event listener from this canvas.
        void RemoveEventListener(const EventListener* eventListener);

        /**
        \brief Posts a 'Quit' event to all event listeners.
        \remarks If at least one event listener returns false within the \c OnQuit callback, the canvas will not quit.
        If all event listeners return true within the \c OnQuit callback, \c ProcessEvents will return false from now on.
        \see EventListener::OnQuit
        \see ProcessEvents
        \see HasQuit
        */
        void PostQuit();

    protected:

        /**
        \brief Called inside the "ProcessEvents" function after all event listeners received the same event.
        \see ProcessEvents
        \see EventListener::OnProcessEvents
        */
        virtual void OnProcessEvents() = 0;

    private:

        std::vector<std::shared_ptr<EventListener>> eventListeners_;
        bool                                        quit_           = false;

};


} // /namespace LLGL


#endif



// ================================================================================
