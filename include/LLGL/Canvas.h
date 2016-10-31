/*
 * Canvas.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_CANVAS_H
#define LLGL_CANVAS_H


#include "Surface.h"
#include "Types.h"
#include "Key.h"
#include <string>
#include <memory>


namespace LLGL
{


//! Canvas descriptor structure.
struct CanvasDescriptor
{
    //! Canvas title as UTF16 string.
    std::wstring    title;

    //! Specifies whether the canvas is borderless. This is required for a fullscreen render context.
    bool            borderless = false;
};


/**
\brief Canvas interface for mobile platforms.
\remarks This is the main interface for the windowing system in LLGL on mobile platforms.
The couterpart is the Window interface for desktop platforms.
\see Window
*/
class LLGL_EXPORT Canvas : public Surface
{

    public:

        //! Interface for all canvas event listeners.
        class LLGL_EXPORT EventListener
        {

            public:

                virtual ~EventListener();

            protected:

                friend class Canvas;

                /**
                \brief Send when the canvas events are about to be polled. The event listeners receive this event before the canvas itself.
                \see Canvas::OnProcessEvents
                */
                virtual void OnProcessEvents(Canvas& sender);

        };

        /* --- Common --- */

        virtual ~Canvas();

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

        //! This default implementation ignores the video mode descriptor completely and always return false.
        bool AdaptForVideoMode(VideoModeDescriptor& videoModeDesc) override;

        //! Processes the events for this canvas (i.e. touch input, key presses etc.).
        void ProcessEvents();

        /* --- Event handling --- */

        //! Adds a new event listener to this canvas.
        void AddEventListener(const std::shared_ptr<EventListener>& eventListener);

        //! Removes the specified event listener from this canvas.
        void RemoveEventListener(const EventListener* eventListener);

    protected:

        /**
        \briefs Called inside the "ProcessEvents" function after all event listeners received the same event.
        \see ProcessEvents
        \see EventListener::OnProcessEvents
        */
        virtual void OnProcessEvents() = 0;

    private:

        std::vector<std::shared_ptr<EventListener>> eventListeners_;

};


} // /namespace LLGL


#endif



// ================================================================================
