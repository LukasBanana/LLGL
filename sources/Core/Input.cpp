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


#define KEY_IDX(k) (static_cast<unsigned char>(k))

Input::Input()
{
    InitArray(keyPressed_);
    InitArray(keyDown_);
    InitArray(keyUp_);
}

bool Input::KeyPressed(Key keyCode) const
{
    return keyPressed_[KEY_IDX(keyCode)];
}

bool Input::KeyDown(Key keyCode) const
{
    return keyDown_[KEY_IDX(keyCode)];
}

bool Input::KeyUp(Key keyCode) const
{
    return keyUp_[KEY_IDX(keyCode)];
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

void Input::InitArray(KeyStateArray& keyStates)
{
    std::fill(keyStates.begin(), keyStates.end(), false);
}

void Input::OnReset(Window& sender)
{
    mouseMotion_ = { 0, 0 };
    keyDownTracker_.Reset(keyDown_);
    keyUpTracker_.Reset(keyUp_);
}

void Input::OnKeyDown(Window& sender, Key keyCode)
{
    auto idx = KEY_IDX(keyCode);

    /* Store key hit state */
    if (!keyPressed_[idx])
    {
        keyDown_[idx] = true;
        keyDownTracker_.Add(keyCode);
    }

    /* Store key pressed state */
    keyPressed_[idx] = true;
}

void Input::OnKeyUp(Window& sender, Key keyCode)
{
    auto idx = KEY_IDX(keyCode);

    /* Store key released state */
    keyUp_[idx] = true;
    keyUpTracker_.Add(keyCode);

    /* Reset key pressed state */
    keyPressed_[idx] = false;
}

void Input::OnLocalMotion(Window& sender, const Point& position)
{
    mousePosition_ = position;
}

void Input::OnGlobalMotion(Window& sender, const Point& motion)
{
    mouseMotion_ = motion;
}

void Input::KeyTracker::Add(Key keyCode)
{
    if (resetCount < maxCount)
        keys[resetCount++] = keyCode;
}

void Input::KeyTracker::Reset(KeyStateArray& keyStates)
{
    while (resetCount > 0)
    {
        --resetCount;
        auto idx = KEY_IDX(keys[resetCount]);
        keyStates[idx] = false;
    }
}

#undef KEY_IDX


} // /namespace LLGL



// ================================================================================
