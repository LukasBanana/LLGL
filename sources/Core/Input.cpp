/*
 * Input.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Input.h>
#include <algorithm>


namespace LLGL
{


Input::Input()
{
    std::fill(keyStates_.begin(), keyStates_.end(), false);
}

#define KEY_IDX(k) static_cast<unsigned char>(k)

void Input::OnKeyDown(Key keyCode)
{
    keyStates_[KEY_IDX(keyCode)] = true;
}

void Input::OnKeyUp(Key keyCode)
{
    keyStates_[KEY_IDX(keyCode)] = false;
}

void Input::OnLocalMotion(int x, int y)
{
    mouseX_ = x;
    mouseY_ = y;
}

void Input::OnGlobalMotion(int dx, int dy)
{
    mouseDX_ = dx;
    mouseDY_ = dy;
}

void Input::Reset()
{
    mouseDX_ = 0;
    mouseDY_ = 0;
}

bool Input::KeyPressed(Key keyCode) const
{
    return keyStates_[KEY_IDX(keyCode)];
}

#undef KEY_IDX

void Input::GetMousePosition(int& x, int& y) const
{
    x = mouseX_;
    y = mouseY_;
}

void Input::GetMouseMotion(int& dx, int& dy) const
{
    dx = mouseDX_;
    dy = mouseDY_;
}


} // /namespace LLGL



// ================================================================================
