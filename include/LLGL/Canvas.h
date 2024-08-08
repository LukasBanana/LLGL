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
#include <LLGL/Deprecated.h>
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

                //! \deprecated Since 0.04b; Use OnDestroy instead to detect when the canvas is about to be destroyed!
                LLGL_DEPRECATED("Deprecated since 0.04b; Use OnDestroy instead!", "OnDestroy")
                virtual void OnQuit(Canvas& sender, bool& veto);

                //! Sent when the canvas is initialized or re-initialized.
                virtual void OnInit(Canvas& sender);

                /**
                \brief Sent when the canvas' native object is about to be destroyed.
                \remarks The Canvas instance itself may still remain active and receive a subsequent OnInit event to re-initialize the native object.
                */
                virtual void OnDestroy(Canvas& sender);

                /**
                \brief Sent when the canvas must redraw its content.
                \param[in] sender Specifies the sender of this event.
                */
                virtual void OnDraw(Canvas& sender);

                //! Sent when the canvas has been resized. This can also happen when the orientation has changed.
                virtual void OnResize(Canvas& sender, const Extent2D& clientAreaSize);

                //! Sent when a tap gesture has been recognized only including the location within the canvas.
                virtual void OnTapGesture(Canvas& sender, const Offset2D& position, std::uint32_t numTouches);

                //! \deprecated Since 0.04b; Use the second version of OnPanGesture() with the EventAction parameter instead!
                LLGL_DEPRECATED("This version of OnPanGesture() is deprecated since 0.04b; Use the second version with the EventAction parameter instead!")
                virtual void OnPanGesture(Canvas& sender, const Offset2D& position, std::uint32_t numTouches, float dx, float dy);

                //! Sent when a pan gesture has been recognized. Includes X and Y deltas for movement.
                virtual void OnPanGesture(Canvas& sender, const Offset2D& position, std::uint32_t numTouches, float dx, float dy, EventAction action);

                //! Sent when a key (from device button) has been pushed.
                virtual void OnKeyDown(Canvas& sender, Key keyCode);

                //! Sent when a key (from device button) has been released.
                virtual void OnKeyUp(Canvas& sender, Key keyCode);

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

    public:

        //! \deprecated Since 0.04b; Write a custom 'quit' state for your app instead!
        LLGL_DEPRECATED("Deprecated since 0.04b; Use a custom state instead!")
        virtual bool HasQuit() const;

        //! This default implementation ignores the video mode descriptor completely and always return false.
        bool AdaptForVideoMode(Extent2D* resolution, bool* fullscreen) override;

        //! Always returns Display::GetPrimary.
        Display* FindResidentDisplay() const override final;

    public:

        /**
        \brief Sets a raw pointer to some user defined data. The initial value is null.
        \remarks This can be used to quickly associate an instance of this class with custom data, especially during event handling.
        */
        void SetUserData(void* userData);

        /**
        \brief Returns the raw pointer that was previously set with SetUserData. The initial value is null.
        \see SetUserData
        */
        void* GetUserData() const;

        //! Adds a new event listener to this canvas.
        void AddEventListener(const std::shared_ptr<EventListener>& eventListener);

        //! Removes the specified event listener from this canvas.
        void RemoveEventListener(const EventListener* eventListener);

        //! \deprecated Since 0.04b; Use PostDestroy instead.
        LLGL_DEPRECATED("Deprecated since 0.04b; Use PostDestroy instead to signal the canvas is about to be destroyed.", "PostDestroy")
        void PostQuit();

        /**
        \brief Posts a signal that the canvas is initialized or re-initialized.
        \remarks A canvas can not only be initialized when the app is launched, but also when the app is resumed, although this is platform dependent.
        On Android, this will be signaled on the \c APP_CMD_INIT_WINDOW command.
        \see EventListener::OnInit
        */
        void PostInit();

        /**
        \brief Posts a signal that the canvas is about to be destroyed.
        \remarks A canvas can not only be destroyed when the app is about to close, but also when the app is paused, although this is platform dependent.
        On Android, this will be signaled on the \c APP_CMD_TERM_WINDOW command.
        \see EventListener::OnDestroy
        */
        void PostDestroy();

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

        //! \deprecated Since 0.04b; Use the second version of PostPanGesture() with the EventAction parameter instead!
        LLGL_DEPRECATED("This version of PostPanGesture() is deprecated since 0.04b; Use the second version with the EventAction parameter instead!")
        void PostPanGesture(const Offset2D& position, std::uint32_t numTouches, float dx, float dy);

        /**
        \brief Posts a pan gesture event to all event listeners.
        \see EventListener::OnPanGesture
        */
        void PostPanGesture(const Offset2D& position, std::uint32_t numTouches, float dx, float dy, EventAction action);

        /**
        \brief Posts a keycode event from a device button that has been pushed down.
        \see EventListener::OnKeyDown
        */
        void PostKeyDown(Key keyCode);

        /**
        \brief Posts a keycode event from a device button that has been released.
        \see EventListener::OnKeyUp
        */
        void PostKeyUp(Key keyCode);

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
