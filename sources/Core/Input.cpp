/*
 * Input.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Input.h>
#include <LLGL/TypeInfo.h>
#include <LLGL/Window.h>
#include <LLGL/Canvas.h>
#include <string.h>
#include <algorithm>


namespace LLGL
{


#define KEY_IDX(k) (static_cast<std::uint8_t>(k))


/*
 * WindowEventListener class
 */

class Input::WindowEventListener final : public Window::EventListener
{

    public:

        WindowEventListener(Input& input) :
            input_ { input }
        {
        }

    private:

        void OnProcessEvents(Window& sender) override
        {
            /* Reset all input states to make room for next recordings */
            input_.wheelMotion_ = 0;
            input_.mouseMotion_ = { 0, 0 };

            input_.keyDownTracker_.Reset(input_.keyDown_);
            input_.keyDownRepeatedTracker_.Reset(input_.keyDownRepeated_);
            input_.keyUpTracker_.Reset(input_.keyUp_);

            ::memset(input_.doubleClick_, 0, sizeof(input_.doubleClick_));

            input_.chars_.clear();
        }

        void OnKeyDown(Window& sender, Key keyCode) override
        {
            auto idx = KEY_IDX(keyCode);

            if (!input_.keyPressed_[idx])
            {
                /* Increase 'any'-key counter and store key state */
                if (input_.anyKeyCount_++ == 0)
                {
                    /* Store key state for 'any'-key */
                    input_.keyDown_[KEY_IDX(Key::Any)] = true;
                    input_.keyDownTracker_.Add(Key::Any);
                    input_.keyPressed_[KEY_IDX(Key::Any)] = true;
                }

                /* Store key hit state */
                input_.keyDown_[idx] = true;
                input_.keyDownTracker_.Add(keyCode);
            }

            /* Store key pressed state */
            input_.keyPressed_[idx] = true;

            /* Store repeated key hit state */
            input_.keyDownRepeated_[idx] = true;
            input_.keyDownRepeatedTracker_.Add(keyCode);
        }

        void OnKeyUp(Window& sender, Key keyCode) override
        {
            /* Store key released state */
            input_.keyUp_[KEY_IDX(keyCode)] = true;
            input_.keyUpTracker_.Add(keyCode);

            /* Store key released state for 'any'-key */
            input_.keyUp_[KEY_IDX(Key::Any)] = true;
            input_.keyUpTracker_.Add(Key::Any);

            /* Increase 'any'-key counter and store key state */
            if (input_.anyKeyCount_ > 0)
            {
                input_.anyKeyCount_--;
                if (input_.anyKeyCount_ == 0)
                    input_.keyPressed_[KEY_IDX(Key::Any)] = false;
            }

            /* Reset key pressed state */
            input_.keyPressed_[KEY_IDX(keyCode)] = false;
        }

        void OnDoubleClick(Window& sender, Key keyCode) override
        {
            switch (keyCode)
            {
                case Key::LButton:
                    input_.doubleClick_[0] = true;
                    break;
                case Key::RButton:
                    input_.doubleClick_[1] = true;
                    break;
                case Key::MButton:
                    input_.doubleClick_[2] = true;
                    break;
                default:
                    break;
            }
        }

        void OnChar(Window& sender, wchar_t chr) override
        {
            input_.chars_ += chr;
        }

        void OnWheelMotion(Window& sender, int motion) override
        {
            input_.wheelMotion_ += motion;
        }

        void OnLocalMotion(Window& sender, const Offset2D& position) override
        {
            input_.mousePosition_ = position;
        }

        void OnGlobalMotion(Window& sender, const Offset2D& motion) override
        {
            input_.mouseMotion_.x += motion.x;
            input_.mouseMotion_.y += motion.y;
        }

        void OnLostFocus(Window& sender) override
        {
            /* Reset all 'key-pressed' states */
            Input::InitArray(input_.keyPressed_);
        }

    private:

        Input& input_;

};


/*
 * CanvasEventListener class
 */

class Input::CanvasEventListener final : public Canvas::EventListener
{

    public:

        CanvasEventListener(Input& input) :
            input_ { input }
        {
        }

    private:

        void OnProcessEvents(Canvas& sender) override
        {
            //TODO
        }

    private:

        Input& input_;

};


/*
 * Input class
 */

Input::Input()
{
    Input::InitArray(keyPressed_);
    Input::InitArray(keyDown_);
    Input::InitArray(keyDownRepeated_);
    Input::InitArray(keyUp_);
    ::memset(doubleClick_, 0, sizeof(doubleClick_));
}

Input::Input(Surface& surface) :
    Input {}
{
    Listen(surface);
}

template <typename T>
bool HasEventListenerForSurface(
    std::vector<std::pair<std::shared_ptr<T>, const Surface*>>& eventListeners,
    const Surface&                                              surface)
{
    return
    (
        std::find_if(
            eventListeners.begin(),
            eventListeners.end(),
            [&surface](const std::pair<std::shared_ptr<T>, const Surface*>& entry)
            {
                return (entry.second == &surface);
            }
        ) == eventListeners.end()
    );
}

void Input::Listen(Surface& surface)
{
    if (LLGL::IsInstanceOf<Window>(surface))
    {
        if (HasEventListenerForSurface(windowEventListeners_, surface))
        {
            auto eventListener = std::make_shared<WindowEventListener>(*this);
            windowEventListeners_.push_back({ eventListener, &surface });
            CastTo<Window>(surface).AddEventListener(eventListener);
        }
    }
    if (LLGL::IsInstanceOf<Canvas>(surface))
    {
        if (HasEventListenerForSurface(canvasEventListeners_, surface))
        {
            auto eventListener = std::make_shared<CanvasEventListener>(*this);
            canvasEventListeners_.push_back({ eventListener, &surface });
            CastTo<Canvas>(surface).AddEventListener(eventListener);
        }
    }
}

void Input::Drop(Surface& surface)
{
    if (LLGL::IsInstanceOf<Window>(surface))
    {
        for (auto it = windowEventListeners_.begin(); it != windowEventListeners_.end(); ++it)
        {
            if (it->second == &surface)
            {
                CastTo<Window>(surface).RemoveEventListener(it->first.get());
                windowEventListeners_.erase(it);
                break;
            }
        }
    }
    if (LLGL::IsInstanceOf<Canvas>(surface))
    {
        for (auto it = canvasEventListeners_.begin(); it != canvasEventListeners_.end(); ++it)
        {
            if (it->second == &surface)
            {
                CastTo<Canvas>(surface).RemoveEventListener(it->first.get());
                canvasEventListeners_.erase(it);
                break;
            }
        }
    }
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
 * ======= Private: =======
 */

void Input::InitArray(KeyStateArray& keyStates)
{
    ::memset(keyStates, 0, sizeof(KeyStateArray));
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
