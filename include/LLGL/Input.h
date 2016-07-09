/*
 * Input.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_INPUT_H__
#define __LLGL_INPUT_H__


#include <LLGL/Window.h>
#include <array>


namespace LLGL
{


class LLGL_EXPORT Input : public Window::Listener
{

    public:

        Input();

        void OnKeyDown(Key keyCode) override;
        void OnKeyUp(Key keyCode) override;

        void OnLocalMotion(int x, int y) override;
        void OnGlobalMotion(int dx, int dy) override;

        /**
        \brief Restes the internal input states.
        \remarks This should be called just before the respective "Window::ProcessEvents" function is called.
        \see Window::ProcessEvents
        */
        void Reset();

        bool KeyPressed(Key keyCode) const;

        void GetMousePosition(int& x, int& y) const;
        void GetMouseMotion(int& dx, int& dy) const;

    private:

        std::array<bool, 256> keyStates_;

        int mouseX_ = 0,
            mouseY_ = 0,
            mouseDX_ = 0,
            mouseDY_ = 0;

};



} // /namespace LLGL


#endif



// ================================================================================
