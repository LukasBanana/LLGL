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


#define KEY_IDX(k) static_cast<unsigned char>(k)

Input::Input()
{
    std::fill(keyStates_.begin(), keyStates_.end(), false);
}

bool Input::KeyPressed(Key keyCode) const
{
    return keyStates_[KEY_IDX(keyCode)];
}

Point Input::GetMousePosition() const
{
    return mousePosition_;
}

Point Input::GetMouseMotion() const
{
    return mouseMotion_;
}


/*
 * ======= Private: =======
 */

void Input::OnReset()
{
    mouseMotion_ = { 0, 0 };
}

void Input::OnKeyDown(Key keyCode)
{
    keyStates_[KEY_IDX(keyCode)] = true;
}

void Input::OnKeyUp(Key keyCode)
{
    keyStates_[KEY_IDX(keyCode)] = false;
}

void Input::OnLocalMotion(const Point& position)
{
    mousePosition_ = position;
}

void Input::OnGlobalMotion(const Point& motion)
{
    mouseMotion_ = motion;
}

#undef KEY_IDX


} // /namespace LLGL



// ================================================================================
