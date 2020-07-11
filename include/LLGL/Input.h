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
#include <array>
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
class LLGL_EXPORT Input : public Window::EventListener
{

        LLGL_DECLARE_INTERFACE( InterfaceID::Input );

    public:

        Input();

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

    protected:

        void OnProcessEvents(Window& sender) override;

        void OnKeyDown(Window& sender, Key keyCode) override;
        void OnKeyUp(Window& sender, Key keyCode) override;

        void OnDoubleClick(Window& sender, Key keyCode) override;

        void OnChar(Window& sender, wchar_t chr) override;

        void OnWheelMotion(Window& sender, int motion) override;

        void OnLocalMotion(Window& sender, const Offset2D& position) override;
        void OnGlobalMotion(Window& sender, const Offset2D& motion) override;

        void OnLostFocus(Window& sender) override;

    private:

        using KeyStateArray = std::array<bool, 256>;

        struct KeyTracker
        {
            static const size_t         maxCount    = 10;
            std::array<Key, maxCount>   keys;
            size_t                      resetCount  = 0;

            void Add(Key keyCode);
            void Reset(KeyStateArray& keyStates);
        };

    private:

        void InitArray(KeyStateArray& keyStates);

    private:

        KeyStateArray       keyPressed_;
        KeyStateArray       keyDown_;
        KeyStateArray       keyDownRepeated_;
        KeyStateArray       keyUp_;

        Offset2D            mousePosition_;
        Offset2D            mouseMotion_;

        int                 wheelMotion_    = 0;

        KeyTracker          keyDownTracker_;
        KeyTracker          keyDownRepeatedTracker_;
        KeyTracker          keyUpTracker_;

        std::array<bool, 3> doubleClick_;

        std::wstring        chars_;

        std::size_t         anyKeyCount_    = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
