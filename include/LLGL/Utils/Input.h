/*
 * Input.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_INPUT_H
#define LLGL_INPUT_H


#include <LLGL/Surface.h>
#include <LLGL/Types.h>
#include <LLGL/Key.h>


namespace LLGL
{


/**
\brief Default window event listener to receive user input.
\remarks This class stores all received user input for a simple evaluation.
However, for efficient evaluation, write your own sub class and only respond to user input when the appropriate callback is invoked.
Here is an example usage:
\code
auto myInput = std::make_shared<LLGL::Input>();
myWindow->AddEventListener(myInput);
while (LLGL::Surface::ProcessEvents()) {
    // Quit main loop when user hit the escape key.
    if (myInput->KeyDown(LLGL::Key::Escape))
        break;

    // Rendering goes here ...
}
\endcode
\todo Make Window::EventListener a member of Input instead of extending it. Also add a member of Canvas::EventListener and add touch event functions.
*/
class LLGL_EXPORT Input : public Interface
{

        LLGL_DECLARE_INTERFACE( InterfaceID::Input );

    public:

        //! Default initializes the input handler without assigning to any surface.
        Input();

        //! Assigns an event listener for this input handler to the specified surface.
        Input(Surface& surface);

        //! Releases the internal data.
        ~Input();

        /**
        \brief Resets the internal state.
        remarks This should be called once \e before Surface::ProcessEvents is invoked.
        \see Surface::ProcessEvents
        */
        void Reset();

        //! Adds an event listener for this input handler to the specified surface.
        void Listen(Surface& surface);

        //! Removes the event listener for this input handler from the specified surface.
        void Drop(Surface& surface);

        //! Returns true if the specified key is currently being pressed down.
        bool KeyPressed(Key keyCode) const;

        //! Returns true if the specified key was pressed down in the previous event processing.
        bool KeyDown(Key keyCode) const;

        //! Returns true if the specified key was pressed down in the previous event processing (this event will be repeated, depending on the paltform settings).
        bool KeyDownRepeated(Key keyCode) const;

        //! Returns true if the specified key was released in the previous event processing.
        bool KeyUp(Key keyCode) const;

        /**
        \brief Returns true if the specified key was double clicked.
        \remarks This can only be true for the key codes: Key::LButton, Key::RButton, and Key::MButton.
        */
        bool KeyDoubleClick(Key keyCode) const;

        //! Returns the local mouse position.
        const Offset2D& GetMousePosition() const;

        //! Returns the global mouse motion.
        const Offset2D& GetMouseMotion() const;

        //! Returns the mouse wheel motion.
        int GetWheelMotion() const;

        //! Returns the entered characters.
        const UTF8String& GetEnteredChars() const;

        //! Returns the number of any keys being pressed.
        unsigned GetAnyKeyCount() const;

    private:

        class WindowEventListener;
        class CanvasEventListener;

        friend WindowEventListener;
        friend CanvasEventListener;

        struct Pimpl;
        Pimpl* pimpl_;

};


} // /namespace LLGL


#endif



// ================================================================================
