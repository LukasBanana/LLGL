/*
 * Input.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_INPUT_H__
#define __LLGL_INPUT_H__


#include <LLGL/Window.h>
#include <LLGL/Types.h>
#include <array>


namespace LLGL
{


class LLGL_EXPORT Input : public Window::EventListener
{

    public:

        Input();

        //! Returns true if the specified key is currently being pressed down.
        bool KeyPressed(Key keyCode) const;

        //! Returns true if the specified key was pressed down in the previous event processing.
        bool KeyDown(Key keyCode) const;

        //! Returns true if the specified key was released in the previous event processing.
        bool KeyUp(Key keyCode) const;

        Point GetMousePosition() const;
        Point GetMouseMotion() const;

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

        void InitArray(KeyStateArray& keyStates);

        void OnReset(Window& sender) override;

        void OnKeyDown(Window& sender, Key keyCode) override;
        void OnKeyUp(Window& sender, Key keyCode) override;

        void OnLocalMotion(Window& sender, const Point& position) override;
        void OnGlobalMotion(Window& sender, const Point& motion) override;

        KeyStateArray           keyPressed_;
        KeyStateArray           keyDown_;
        KeyStateArray           keyUp_;

        Point                   mousePosition_;
        Point                   mouseMotion_;

        KeyTracker              keyDownTracker_;
        KeyTracker              keyUpTracker_;

};


} // /namespace LLGL


#endif



// ================================================================================
