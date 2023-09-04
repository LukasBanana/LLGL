/*
 * Canvas.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_CANVAS_H
#define LLGL_CANVAS_H


#include <LLGL/CanvasFlags.h>
#include <LLGL/Surface.h>
#include <LLGL/Types.h>
#include <LLGL/Key.h>
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
                \brief Sent when the canvas is about to quit.
                \param[in] sender Specifies the sender of this event.
                \param[out] veto Specifies whether to cancel the quit event.
                If set to true, the call to \c PostQuit does not change the state \c sender, only the event listeners get informed.
                If no event listener sets this parameter to true, \c sender is set into 'Quit' state.
                */
                virtual void OnQuit(Canvas& sender, bool& veto);

                /**
                \brief Sent when the canvas must redraw its content.
                \param[in] sender Specifies the sender of this event.
                */
                virtual void OnDraw(Canvas& sender);

                //! Sent when the canvas has been resized. This can also happen when the orientation has changed.
                virtual void OnResize(Canvas& sender, const Extent2D& clientAreaSize);

                //! Sent when a tap gesture has been recognized only including the location within the canvas.
                virtual void OnTapGesture(Canvas& sender, const Offset2D& position, std::uint32_t numTouches);

                //! Sent when a pan gesture has been recognized. Includes X and Y deltas for movement.
                virtual void OnPanGesture(Canvas& sender, const Offset2D& position, std::uint32_t numTouches, float dx, float dy);

        };

    public:

        //! Releases the internal data.
        ~Canvas();

        /* --- Common --- */

        /**
        \brief Creates a platform specific instance of the Canvas interface.
        \return Unique pointer to a new instance of the platform specific Canvas interface or
        null if the platform does not support canvas (such as Windows, Linux, and macOS).
        \remarks For desktop platforms the interface Window can be used.
        \see Window
        */
        static std::unique_ptr<Canvas> Create(const CanvasDescriptor& desc);

        //! Sets the canvas title as UTF-8 string.
        virtual void SetTitle(const UTF8String& title) = 0;

        //! Returns the canvas title as UTF16 string.
        virtual UTF8String GetTitle() const = 0;

        /**
        \brief Returns true if this canvas is in the 'Quit' state.
        \see PostQuit
        */
        virtual bool HasQuit() const;

        //! This default implementation ignores the video mode descriptor completely and always return false.
        bool AdaptForVideoMode(Extent2D* resolution, bool* fullscreen) override;

        //! Always returns Display::GetPrimary.
        Display* FindResidentDisplay() const override final;

        /* --- Event handling --- */

        //! Adds a new event listener to this canvas.
        void AddEventListener(const std::shared_ptr<EventListener>& eventListener);

        //! Removes the specified event listener from this canvas.
        void RemoveEventListener(const EventListener* eventListener);

        /**
        \brief Posts a 'Quit' event to all event listeners.
        \remarks If any of the event listener sets the \c veto flag to false within the \c OnQuit callback, the canvas will \e not be put into 'Quit' state.
        \see EventListener::OnQuit
        \see HasQuit
        */
        void PostQuit();

        /**
        \brief Posts a draw event to all event listeners.
        \see EventListener::OnDraw
        */
        void PostDraw();

        /**
        \brief Posts a resize event to all event listeners.
        \see EventListener::OnResize
        */
        void PostResize(const Extent2D& clientAreaSize);

        /**
        \brief Posts a tap gesture event to all event listeners.
        \see EventListener::OnTapGesture
        */
        void PostTapGesture(const Offset2D& position, std::uint32_t numTouches);

        /**
        \brief Posts a pan gesture event to all event listeners.
        \see EventListener::OnPanGesture
        */
        void PostPanGesture(const Offset2D& position, std::uint32_t numTouches, float dx, float dy);

    protected:

        //! Allocates the internal data.
        Canvas();

    private:

        struct Pimpl;
        Pimpl* pimpl_;

};


} // /namespace LLGL


#endif



// ================================================================================
