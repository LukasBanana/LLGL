/*
 * Input.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Utils/Input.h>
#include <LLGL/TypeInfo.h>
#include <LLGL/Window.h>
#include <LLGL/Canvas.h>
#include <LLGL/Container/Strings.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <memory>


namespace LLGL
{


#define KEY_IDX(k) (static_cast<std::uint8_t>(k))


/*
 * KeyTracker structure
 */

using KeyStateArray = bool[256];
using DoubleClickArray = bool[3];

static void ResetKeyStateArray(KeyStateArray& states)
{
    ::memset(states, 0, sizeof(states));
}

static void ResetDoubleClickArray(DoubleClickArray& states)
{
    ::memset(states, 0, sizeof(states));
}

struct KeyTracker
{
    static const size_t maxCount    = 10;

    Key                 keys[maxCount];
    size_t              resetCount  = 0;

    void Add(Key keyCode);
    void Reset(KeyStateArray& keyStates);
};

void KeyTracker::Add(Key keyCode)
{
    if (resetCount < maxCount)
        keys[resetCount++] = keyCode;
}

void KeyTracker::Reset(KeyStateArray& keyStates)
{
    while (resetCount > 0)
    {
        --resetCount;
        auto idx = KEY_IDX(keys[resetCount]);
        keyStates[idx] = false;
    }
}



/*
 * Pimpl structure
 */

template <typename T>
struct EventListenerSurfacePair
{
    std::shared_ptr<T>  eventListener;
    Surface*            surface;
};

struct Input::Pimpl
{
    KeyStateArray   keyPressed;
    KeyStateArray   keyDown;
    KeyStateArray   keyDownRepeated;
    KeyStateArray   keyUp;
    bool            doubleClick[3];

    KeyTracker      keyDownTracker;
    KeyTracker      keyDownRepeatedTracker;
    KeyTracker      keyUpTracker;

    Offset2D        mousePosition;
    Offset2D        mouseMotion;

    int             wheelMotion             = 0;
    unsigned        anyKeyCount             = 0;
    UTF8String      chars;

    std::vector<EventListenerSurfacePair<WindowEventListener>>
                    windowEventListeners;

    std::vector<EventListenerSurfacePair<CanvasEventListener>>
                    canvasEventListeners;
};


/*
 * WindowEventListener class
 */

class Input::WindowEventListener final : public Window::EventListener
{

    public:

        WindowEventListener(Input::Pimpl& data) :
            data_ { data }
        {
        }

    private:

        void OnProcessEvents(Window& sender) override
        {
            /* Reset all input states to make room for next recordings */
            data_.wheelMotion = 0;
            data_.mouseMotion = { 0, 0 };

            data_.keyDownTracker.Reset(data_.keyDown);
            data_.keyDownRepeatedTracker.Reset(data_.keyDownRepeated);
            data_.keyUpTracker.Reset(data_.keyUp);

            ResetDoubleClickArray(data_.doubleClick);

            data_.chars.clear();
        }

        void OnKeyDown(Window& sender, Key keyCode) override
        {
            auto idx = KEY_IDX(keyCode);

            if (!data_.keyPressed[idx])
            {
                /* Increase 'any'-key counter and store key state */
                if (data_.anyKeyCount++ == 0)
                {
                    /* Store key state for 'any'-key */
                    data_.keyDown[KEY_IDX(Key::Any)] = true;
                    data_.keyDownTracker.Add(Key::Any);
                    data_.keyPressed[KEY_IDX(Key::Any)] = true;
                }

                /* Store key hit state */
                data_.keyDown[idx] = true;
                data_.keyDownTracker.Add(keyCode);
            }

            /* Store key pressed state */
            data_.keyPressed[idx] = true;

            /* Store repeated key hit state */
            data_.keyDownRepeated[idx] = true;
            data_.keyDownRepeatedTracker.Add(keyCode);
        }

        void OnKeyUp(Window& sender, Key keyCode) override
        {
            /* Store key released state */
            data_.keyUp[KEY_IDX(keyCode)] = true;
            data_.keyUpTracker.Add(keyCode);

            /* Store key released state for 'any'-key */
            data_.keyUp[KEY_IDX(Key::Any)] = true;
            data_.keyUpTracker.Add(Key::Any);

            /* Increase 'any'-key counter and store key state */
            if (data_.anyKeyCount > 0)
            {
                data_.anyKeyCount--;
                if (data_.anyKeyCount == 0)
                    data_.keyPressed[KEY_IDX(Key::Any)] = false;
            }

            /* Reset key pressed state */
            data_.keyPressed[KEY_IDX(keyCode)] = false;
        }

        void OnDoubleClick(Window& sender, Key keyCode) override
        {
            switch (keyCode)
            {
                case Key::LButton:
                    data_.doubleClick[0] = true;
                    break;
                case Key::RButton:
                    data_.doubleClick[1] = true;
                    break;
                case Key::MButton:
                    data_.doubleClick[2] = true;
                    break;
                default:
                    break;
            }
        }

        void OnChar(Window& sender, wchar_t chr) override
        {
            data_.chars += chr;
        }

        void OnWheelMotion(Window& sender, int motion) override
        {
            data_.wheelMotion += motion;
        }

        void OnLocalMotion(Window& sender, const Offset2D& position) override
        {
            data_.mousePosition = position;
        }

        void OnGlobalMotion(Window& sender, const Offset2D& motion) override
        {
            data_.mouseMotion.x += motion.x;
            data_.mouseMotion.y += motion.y;
        }

        void OnLostFocus(Window& sender) override
        {
            /* Reset all 'key-pressed' states */
            ResetKeyStateArray(data_.keyPressed);
        }

    private:

        Input::Pimpl& data_;

};


/*
 * CanvasEventListener class
 */

class Input::CanvasEventListener final : public Canvas::EventListener
{

    public:

        CanvasEventListener(Input::Pimpl& data)// :
            //data_ { data }
        {
        }

    private:

        void OnProcessEvents(Canvas& sender) override
        {
            //TODO
        }

    private:

        //Input::Pimpl& data_;

};


/*
 * Input class
 */

Input::Input() :
    pimpl_ { new Pimpl{} }
{
    ResetKeyStateArray(pimpl_->keyPressed);
    ResetKeyStateArray(pimpl_->keyDown);
    ResetKeyStateArray(pimpl_->keyDownRepeated);
    ResetKeyStateArray(pimpl_->keyUp);
    ::memset(pimpl_->doubleClick, 0, sizeof(pimpl_->doubleClick));
}

Input::Input(Surface& surface) :
    Input {}
{
    Listen(surface);
}

Input::~Input()
{
    for (const auto& windowEventListener : pimpl_->windowEventListeners)
        CastTo<Window>(windowEventListener.surface)->RemoveEventListener(windowEventListener.eventListener.get());
    for (const auto& canvasEventListener : pimpl_->canvasEventListeners)
        CastTo<Canvas>(canvasEventListener.surface)->RemoveEventListener(canvasEventListener.eventListener.get());
    delete pimpl_;
}

template <typename T>
bool HasEventListenerForSurface(
    std::vector<EventListenerSurfacePair<T>>&   eventListeners,
    const Surface*                              surface)
{
    return
    (
        std::find_if(
            eventListeners.begin(),
            eventListeners.end(),
            [surface](const EventListenerSurfacePair<T>& entry)
            {
                return (entry.surface == surface);
            }
        ) == eventListeners.end()
    );
}

void Input::Listen(Surface& surface)
{
    if (LLGL::IsInstanceOf<Window>(surface))
    {
        if (HasEventListenerForSurface(pimpl_->windowEventListeners, &surface))
        {
            auto eventListener = std::make_shared<WindowEventListener>(*pimpl_);
            pimpl_->windowEventListeners.push_back({ eventListener, &surface });
            CastTo<Window>(surface).AddEventListener(eventListener);
        }
    }
    if (LLGL::IsInstanceOf<Canvas>(surface))
    {
        if (HasEventListenerForSurface(pimpl_->canvasEventListeners, &surface))
        {
            auto eventListener = std::make_shared<CanvasEventListener>(*pimpl_);
            pimpl_->canvasEventListeners.push_back({ eventListener, &surface });
            CastTo<Canvas>(surface).AddEventListener(eventListener);
        }
    }
}

void Input::Drop(Surface& surface)
{
    if (LLGL::IsInstanceOf<Window>(surface))
    {
        for (auto it = pimpl_->windowEventListeners.begin(); it != pimpl_->windowEventListeners.end(); ++it)
        {
            if (it->surface == &surface)
            {
                CastTo<Window>(surface).RemoveEventListener(it->eventListener.get());
                pimpl_->windowEventListeners.erase(it);
                break;
            }
        }
    }
    if (LLGL::IsInstanceOf<Canvas>(surface))
    {
        for (auto it = pimpl_->canvasEventListeners.begin(); it != pimpl_->canvasEventListeners.end(); ++it)
        {
            if (it->surface == &surface)
            {
                CastTo<Canvas>(surface).RemoveEventListener(it->eventListener.get());
                pimpl_->canvasEventListeners.erase(it);
                break;
            }
        }
    }
}

bool Input::KeyPressed(Key keyCode) const
{
    return pimpl_->keyPressed[KEY_IDX(keyCode)];
}

bool Input::KeyDown(Key keyCode) const
{
    return pimpl_->keyDown[KEY_IDX(keyCode)];
}

bool Input::KeyDownRepeated(Key keyCode) const
{
    return pimpl_->keyDownRepeated[KEY_IDX(keyCode)];
}

bool Input::KeyUp(Key keyCode) const
{
    return pimpl_->keyUp[KEY_IDX(keyCode)];
}

bool Input::KeyDoubleClick(Key keyCode) const
{
    switch (keyCode)
    {
        case Key::LButton: return pimpl_->doubleClick[0];
        case Key::RButton: return pimpl_->doubleClick[1];
        case Key::MButton: return pimpl_->doubleClick[2];
        default:           return false;
    }
    return false;
}

const Offset2D& Input::GetMousePosition() const
{
    return pimpl_->mousePosition;
}

const Offset2D& Input::GetMouseMotion() const
{
    return pimpl_->mouseMotion;
}

int Input::GetWheelMotion() const
{
    return pimpl_->wheelMotion;
}

const UTF8String& Input::GetEnteredChars() const
{
    return pimpl_->chars;
}

unsigned Input::GetAnyKeyCount() const
{
    return pimpl_->anyKeyCount;
}

#undef KEY_IDX


} // /namespace LLGL



// ================================================================================
