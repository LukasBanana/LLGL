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
    std::fill(doubleClick_.begin(), doubleClick_.end(), false);
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

bool Input::KeyDoubleClick(Key keyCode) const
{
    switch (keyCode)
    {
        case Key::LButton: return doubleClick_[0];
        case Key::RButton: return doubleClick_[1];
        case Key::MButton: return doubleClick_[2];
        default:           return false;
    }
    return false;
}


/*
 * ======= Private: =======
 */

void Input::InitArray(KeyStateArray& keyStates)
{
    std::fill(keyStates.begin(), keyStates.end(), false);
}

void Input::OnProcessEvents(Window& sender)
{
    wheelMotion_ = 0;
    mouseMotion_ = { 0, 0 };

    keyDownTracker_.Reset(keyDown_);
    keyUpTracker_.Reset(keyUp_);

    std::fill(doubleClick_.begin(), doubleClick_.end(), false);

    chars_.clear();
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

void Input::OnDoubleClick(Window& sender, Key keyCode)
{
    switch (keyCode)
    {
        case Key::LButton: doubleClick_[0] = true; break;
        case Key::RButton: doubleClick_[1] = true; break;
        case Key::MButton: doubleClick_[2] = true; break;
        default:                                   break;
    }
}

void Input::OnChar(Window& sender, wchar_t chr)
{
    chars_+= chr;
}

void Input::OnWheelMotion(Window& sender, int motion)
{
    wheelMotion_ += motion;
}

void Input::OnLocalMotion(Window& sender, const Point& position)
{
    mousePosition_ = position;
}

void Input::OnGlobalMotion(Window& sender, const Point& motion)
{
    mouseMotion_ += motion;
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
