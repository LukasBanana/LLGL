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
The counterpart is the Window interface for desktop platforms.
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
                \brief Returns true if this event listener consumes its input events. By default true.
                \remarks This is primarily used for WebAssembly where the canvas can decide to consume or forward all input events to the browser.
                By default, all events are consumed by the event listeners to prevent unintentional browser scrolling, text selection etc.
                */
                virtual bool IsConsumeEventsEnabled() const;

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
                virtual void OnTapGesture(Canvas& sender, Offset2D position, std::uint32_t touchIndex, std::uint32_t numTouches);

                //! Sent when a pan gesture has been recognized. Includes X and Y deltas for movement.
                virtual void OnPanGesture(Canvas& sender, Offset2D position, std::uint32_t touchIndex, std::uint32_t numTouches, float dX, float dY, EventAction action);

                //! Sent when a key (from device button) has been pushed.
                virtual void OnKeyDown(Canvas& sender, Key keyCode);

                //! Sent when a key (from device button) has been released.
                virtual void OnKeyUp(Canvas& sender, Key keyCode);

                /**
                \brief Sent when a mouse button has been double clicked.
                \note Only supported on: WebAssembly.
                */
                virtual void OnDoubleClick(Canvas& sender, Key keyCode);

                /**
                \brief Sent when the mouse wheel has been moved on the sender window.
                \note Only supported on: WebAssembly.
                */
                virtual void OnWheelMotion(Canvas& sender, float dX, float dY);

                /**
                \brief Sent when the mouse has been moved on the sender canvas.
                \param[in] sender Canvas which sent this event.
                \param[in] position Mouse position relative to the canvas' client area.
                \param[in] dX Motion vector on X-axis.
                \param[in] dY Motion vector on Y-axis.
                \note Only supported on: WebAssembly.
                */
                virtual void OnMouseMotion(Canvas& sender, Offset2D position, float dX, float dY);

                //! \deprecated Since 0.05b; Use the new version with \c touchIndex parameter.
                LLGL_DEPRECATED("This version of `OnTapGesture()` is deprecated since 0.05b; Use the new version with `touchIndex` parameter instead!")
                virtual void OnTapGesture(Canvas& sender, const Offset2D& position, std::uint32_t numTouches);

                //! \deprecated Since 0.05b; Use the new version with \c touchIndex parameter.
                LLGL_DEPRECATED("This version of `OnPanGesture()` is deprecated since 0.05b; Use the new version with `touchIndex` parameter instead!")
                virtual void OnPanGesture(Canvas& sender, const Offset2D& position, std::uint32_t numTouches, float dX, float dY, EventAction action);

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
        \return True if the event was consumed by at least one registered event handler.
        \see EventListener::OnTapGesture
        */
        bool PostTapGesture(Offset2D position, std::uint32_t touchIndex, std::uint32_t numTouches);

        /**
        \brief Posts a pan gesture event to all event listeners.
        \return True if the event was consumed by at least one registered event handler.
        \see EventListener::OnPanGesture
        */
        bool PostPanGesture(Offset2D position, std::uint32_t touchIndex, std::uint32_t numTouches, float dx, float dy, EventAction action);

        /**
        \brief Posts a keycode event from a device button that has been pushed down.
        \return True if the event was consumed by at least one registered event handler.
        \see EventListener::OnKeyDown
        */
        bool PostKeyDown(Key keyCode);

        /**
        \brief Posts a keycode event from a device button that has been released.
        \return True if the event was consumed by at least one registered event handler.
        \see EventListener::OnKeyUp
        */
        bool PostKeyUp(Key keyCode);

        /**
        \brief Posts a double click event to this canvas.
        \return True if the event was consumed by at least one registered event handler.
        \note Only supported on: WebAssembly.
        \see EventListener::OnDoubleClick
        */
        bool PostDoubleClick(Key keyCode);

        /**
        \brief Posts a mouse wheel motion event to this canvas.
        \return True if the event was consumed by at least one registered event handler.
        \note Only supported on: WebAssembly.
        \see EventListener::OnWheelMotion
        */
        bool PostWheelMotion(float dx, float dy);

        /**
        \brief Posts a local mouse motion event to this canvas.
        \return True if the event was consumed by at least one registered event handler.
        \note Only supported on: WebAssembly.
        \see EventListener::OnMouseMotion
        */
        bool PostMouseMotion(Offset2D position, float dx, float dy);

        /**
        \deprecated Since 0.05b; Use new version with `touchIndex` parameter.
        \see PostTapGesture(const Offset2D&, std::uint32_t, std::uint32_t)
        */
        LLGL_DEPRECATED("This version of `PostTapGesture()` is deprecated since 0.05b; Use the new version with `touchIndex` parameter!")
        inline void PostTapGesture(const Offset2D& position, std::uint32_t numTouches)
        {
            PostTapGesture(position, 0, numTouches);
        }

        /**
        \deprecated Since 0.05b; Use new version with `touchIndex` parameter.
        \see PostPanGesture(const Offset2D&, std::uint32_t, std::uint32_t, float, float, EventAction)
        */
        LLGL_DEPRECATED("This version of `PostPanGesture()` is deprecated since 0.05b; Use the new version with `touchIndex` parameter!")
        inline void PostPanGesture(const Offset2D& position, std::uint32_t numTouches, float dx, float dy, EventAction action)
        {
            PostPanGesture(position, 0, numTouches, dx, dy, action);
        }

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
