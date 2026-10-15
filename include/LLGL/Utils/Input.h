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
\brief Enumeration for input pointer states.
\remarks This is used to record the current state of touch inputs being pressed down and lifted up.
\see InputState::state
\see Input::Reset
*/
enum class InputState
{
    //! Input is currently inactive.
    Inactive,

    /**
    \brief Input is being activated.
    \remarks This is when a touch pointer is being pressed down.
    After the next call to Input::Reset, it will be either in the Active or Deactivate state.
    */
    Activate,

    /**
    \brief Input is currently active.
    \remarks The mouse cursor on desktop devices is always considered active.
    Use Input::KeyDown, Input::KeyPressed, and Input::KeyUp to determine the specific mouse button states.
    */
    Active,

    /**
    \brief Input is being deactivated.
    \remarks This is when a touch pointer is being lifted up.
    After the next call to Input::Reset, it will be either in the Inactive or Activate state.
    */
    Deactivate,
};

/**
\brief Enumeration for input event handling modes.
\remarks
\see Input::SetInputHandling
*/
enum class InputHandling
{
    Ignore,
    Process,
    ProcessAndConsume,
};

/**
\brief Input pointer (mouse or touch input) structure for generic event handling.
\see Input::GetPointer
*/
struct InputPointer
{
    //! Local offset relative to the canvas.
    Offset2D    position;

    //! Motion vector X-axis.
    float       dX          = 0.0f;

    //! Motion vector Y-axis.
    float       dY          = 0.0f;

    //! Specifies whether this touch input is active, i.e. whether it's currently being pressed down.
    InputState  state       = InputState::Inactive;
};

/**
\brief Default window event listener to receive user input.
\remarks This class stores all received user input for a simple evaluation.
However, for efficient evaluation, write your own sub class and only respond to user input when the appropriate callback is invoked.
Here is an example usage:
\code
LLGL::Input myInput;
myInput.Listen(*myWindow);
while (LLGL::Surface::ProcessEvents()) {
    // Quit main loop when user hit the escape key.
    if (myInput.KeyDown(LLGL::Key::Escape))
        break;

    // Rendering goes here ...

    // Reset input states
    myInput.Reset();
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
        \todo Rename to Tick() or something else to indicate this needs to be called once a frame.
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

        //! Returns true if the specified key was pressed down in the previous event processing (this event will be repeated, depending on the platform settings).
        bool KeyDownRepeated(Key keyCode) const;

        //! Returns true if the specified key was released in the previous event processing.
        bool KeyUp(Key keyCode) const;

        /**
        \brief Returns true if the specified key was double clicked.
        \remarks This can only be true for the key codes: Key::LButton, Key::RButton, and Key::MButton.
        */
        bool KeyDoubleClick(Key keyCode) const;

        /**
        \brief Returns the local position (relative to the client area of the Window or Canvas) of the specified input pointer or null if it's not active.
        \param index Specifies the pointer index. Index 0 is the default for mouse input. Other values are used exclusively for touch input.
        The maximum value index is <code>LLGL_MAX_NUM_INPUT_POINTERS - 1</code>
        \see LLGL_MAX_NUM_INPUT_POINTERS
        */
        const InputPointer& GetPointer(unsigned index) const;

        //! Returns the local mouse position.
        Offset2D GetMousePosition() const;

        //! Returns the global mouse motion.
        Offset2D GetMouseMotion() const;

        //! Returns the mouse wheel motion.
        int GetWheelMotion() const;

        //! Returns the entered characters.
        const UTF8String& GetEnteredChars() const;

        //! Returns the number of any keys being pressed.
        unsigned GetAnyKeyCount() const;

        /**
        \brief Sets the input event handling mode. By default InputHandling::ProcessAndConsume.
        \see Canvas::EventListener::IsConsumeEventsEnabled
        */
        void SetInputHandling(InputHandling inputHandling);

        /**
        \brief Returnst the input event handling mode.
        \see Canvas::EventListener::IsConsumeEventsEnabled
        */
        InputHandling GetInputHandling() const;

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
