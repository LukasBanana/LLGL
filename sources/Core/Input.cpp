/*
 * Input.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Input.h>
#include <algorithm>


namespace LLGL
{


#define KEY_IDX(k) (static_cast<std::uint8_t>(k))

Input::Input()
{
    InitArray(keyPressed_);
    InitArray(keyDown_);
    InitArray(keyDownRepeated_);
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

bool Input::KeyDownRepeated(Key keyCode) const
{
    return keyDownRepeated_[KEY_IDX(keyCode)];
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
 * ======= Protected: =======
 */

void Input::OnProcessEvents(Window& sender)
{
    wheelMotion_ = 0;
    mouseMotion_ = { 0, 0 };

    keyDownTracker_.Reset(keyDown_);
    keyDownRepeatedTracker_.Reset(keyDownRepeated_);
    keyUpTracker_.Reset(keyUp_);

    std::fill(doubleClick_.begin(), doubleClick_.end(), false);

    chars_.clear();
}

void Input::OnKeyDown(Window& sender, Key keyCode)
{
    auto idx = KEY_IDX(keyCode);

    if (!keyPressed_[idx])
    {
        /* Increase 'any'-key counter and store key state */
        if (anyKeyCount_++ == 0)
        {
            /* Store key state for 'any'-key */
            keyDown_[KEY_IDX(Key::Any)] = true;
            keyDownTracker_.Add(Key::Any);
            keyPressed_[KEY_IDX(Key::Any)] = true;
        }

        /* Store key hit state */
        keyDown_[idx] = true;
        keyDownTracker_.Add(keyCode);
    }

    /* Store key pressed state */
    keyPressed_[idx] = true;

    /* Store repeated key hit state */
    keyDownRepeated_[idx] = true;
    keyDownRepeatedTracker_.Add(keyCode);
}

void Input::OnKeyUp(Window& sender, Key keyCode)
{
    /* Store key released state */
    keyUp_[KEY_IDX(keyCode)] = true;
    keyUpTracker_.Add(keyCode);

    /* Store key released state for 'any'-key */
    keyUp_[KEY_IDX(Key::Any)] = true;
    keyUpTracker_.Add(Key::Any);

    /* Increase 'any'-key counter and store key state */
    if (anyKeyCount_ > 0)
    {
        if (--anyKeyCount_ == 0)
            keyPressed_[KEY_IDX(Key::Any)] = false;
    }

    /* Reset key pressed state */
    keyPressed_[KEY_IDX(keyCode)] = false;
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
    chars_ += chr;
}

void Input::OnWheelMotion(Window& sender, int motion)
{
    wheelMotion_ += motion;
}

void Input::OnLocalMotion(Window& sender, const Offset2D& position)
{
    mousePosition_ = position;
}

void Input::OnGlobalMotion(Window& sender, const Offset2D& motion)
{
    mouseMotion_.x += motion.x;
    mouseMotion_.y += motion.y;
}

void Input::OnLostFocus(Window& sender)
{
    /* Reset all 'key-pressed' states */
    InitArray(keyPressed_);
}


/*
 * ======= Private: =======
 */

void Input::InitArray(KeyStateArray& keyStates)
{
    std::fill(keyStates.begin(), keyStates.end(), false);
}


/*
 * KeyTracker structure
 */

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
