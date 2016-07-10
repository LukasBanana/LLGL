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

        bool KeyPressed(Key keyCode) const;

        Point GetMousePosition() const;
        Point GetMouseMotion() const;

    private:

        void OnKeyDown(Key keyCode) override;
        void OnKeyUp(Key keyCode) override;

        void OnLocalMotion(const Point& position) override;
        void OnGlobalMotion(const Point& motion) override;

        void OnReset() override;

        std::array<bool, 256> keyStates_;

        Point mousePosition_, mouseMotion_;

};


} // /namespace LLGL


#endif



// ================================================================================
