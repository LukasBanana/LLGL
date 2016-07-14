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


class LLGL_EXPORT Input : public Window::Listener
{

    public:

        Input();

        //! Returns true if the specified key is currently being pressed down.
        bool KeyPressed(Key keyCode) const;
        //! Returns true if the specified key was hit in the previous event processing.
        bool KeyHit(Key keyCode) const;
        //! Returns true if the specified key was released in the previous event processing.
        bool KeyReleased(Key keyCode) const;

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

        void OnKeyDown(Key keyCode) override;
        void OnKeyUp(Key keyCode) override;

        void OnLocalMotion(const Point& position) override;
        void OnGlobalMotion(const Point& motion) override;

        void OnReset() override;

        KeyStateArray           keyPressed_;
        KeyStateArray           keyHit_;
        KeyStateArray           keyReleased_;

        Point                   mousePosition_;
        Point                   mouseMotion_;

        KeyTracker              keyHitTracker_;
        KeyTracker              keyReleasedTracker_;

};


} // /namespace LLGL


#endif



// ================================================================================
