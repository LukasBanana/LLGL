/*
 * Input.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_INPUT_H
#define LLGL_INPUT_H


#include <LLGL/Window.h>
#include <LLGL/Types.h>
#include <string>


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
while (myWindow->ProcessEvents()) {
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
        inline const Offset2D& GetMousePosition() const
        {
            return mousePosition_;
        }

        //! Returns the global mouse motion.
        inline const Offset2D& GetMouseMotion() const
        {
            return mouseMotion_;
        }

        //! Returns the mouse wheel motion.
        inline int GetWheelMotion() const
        {
            return wheelMotion_;
        }

        //! Returns the entered characters.
        inline const std::wstring& GetEnteredChars() const
        {
            return chars_;
        }

        //! Returns the number of any keys being pressed.
        inline std::size_t GetAnyKeyCount() const
        {
            return anyKeyCount_;
        }

    private:

        using KeyStateArray = bool[256];

        struct KeyTracker
        {
            static const size_t maxCount    = 10;
            Key                 keys[maxCount];
            size_t              resetCount  = 0;

            void Add(Key keyCode);
            void Reset(KeyStateArray& keyStates);
        };

    private:

        static void InitArray(KeyStateArray& keyStates);

    private:

        class WindowEventListener;
        class CanvasEventListener;

        friend WindowEventListener;
        friend CanvasEventListener;

        KeyStateArray   keyPressed_;
        KeyStateArray   keyDown_;
        KeyStateArray   keyDownRepeated_;
        KeyStateArray   keyUp_;

        Offset2D        mousePosition_;
        Offset2D        mouseMotion_;

        int             wheelMotion_    = 0;

        KeyTracker      keyDownTracker_;
        KeyTracker      keyDownRepeatedTracker_;
        KeyTracker      keyUpTracker_;

        bool            doubleClick_[3];

        std::wstring    chars_;

        std::size_t     anyKeyCount_    = 0;

        std::vector<std::pair<std::shared_ptr<WindowEventListener>, const Surface*>> windowEventListeners_;
        std::vector<std::pair<std::shared_ptr<CanvasEventListener>, const Surface*>> canvasEventListeners_;

};


} // /namespace LLGL


#endif



// ================================================================================
