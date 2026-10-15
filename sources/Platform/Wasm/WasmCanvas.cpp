/*
 * WasmCanvas.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WasmCanvas.h"
#include "WasmKeyCodes.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Display.h>
#include <LLGL/Utils/ForRange.h>
#include "../../Core/CoreUtils.h"
#include <cmath>


namespace LLGL
{


/*
 * Surface class
 */

bool Surface::ProcessEvents()
{
    return true; // dummy - handled by web browser
}


/*
 * Canvas class
 */

std::unique_ptr<Canvas> Canvas::Create(const CanvasDescriptor& desc)
{
    return MakeUnique<WasmCanvas>(desc);
}


/*
 * WasmCanvas class
 */

WasmCanvas::WasmCanvas(const CanvasDescriptor& desc)
{
    CreateEmscriptenCanvas(desc);
}

bool WasmCanvas::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(NativeHandle))
    {
        auto* handle = static_cast<NativeHandle*>(nativeHandle);
        handle->canvas = canvas_;
        return true;
    }
    return false;
}

Extent2D WasmCanvas::GetContentSize() const
{
    int width = 0, height = 0;
    emscripten_get_canvas_element_size(canvasSelector_.c_str(), &width, &height);
    return Extent2D
    {
        static_cast<std::uint32_t>(width),
        static_cast<std::uint32_t>(height)
    };
}

void WasmCanvas::SetTitle(const UTF8String& title)
{
    emscripten_set_window_title(title.c_str());
}

UTF8String WasmCanvas::GetTitle() const
{
    return emscripten_get_window_title();
}


/*
 * ======= Private: =======
 */

/*static const char* EmscriptenResultToString(EMSCRIPTEN_RESULT result)
{
    switch (result)
    {
        case EMSCRIPTEN_RESULT_SUCCESS:             return "EMSCRIPTEN_RESULT_SUCCESS";
        case EMSCRIPTEN_RESULT_DEFERRED:            return "EMSCRIPTEN_RESULT_DEFERRED";
        case EMSCRIPTEN_RESULT_NOT_SUPPORTED:       return "EMSCRIPTEN_RESULT_NOT_SUPPORTED";
        case EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED: return "EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED";
        case EMSCRIPTEN_RESULT_INVALID_TARGET:      return "EMSCRIPTEN_RESULT_INVALID_TARGET";
        case EMSCRIPTEN_RESULT_UNKNOWN_TARGET:      return "EMSCRIPTEN_RESULT_UNKNOWN_TARGET";
        case EMSCRIPTEN_RESULT_INVALID_PARAM:       return "EMSCRIPTEN_RESULT_INVALID_PARAM";
        case EMSCRIPTEN_RESULT_FAILED:              return "EMSCRIPTEN_RESULT_FAILED";
        case EMSCRIPTEN_RESULT_NO_DATA:             return "EMSCRIPTEN_RESULT_NO_DATA";
    }
    return "Unknown EMSCRIPTEN_RESULT!";
}*/

/*static int EmscriptenKeyeventToCharcode(int eventType, const EmscriptenKeyboardEvent* event)
{
    // Only KeyPress events carry a charCode. For KeyDown and KeyUp events, these don't seem to be present yet, until later when the KeyDown
    // is transformed to KeyPress. Sometimes it can be useful to already at KeyDown time to know what the charCode of the resulting
    // KeyPress will be. The following attempts to do this:
    if (eventType == EMSCRIPTEN_EVENT_KEYPRESS && event->which) return event->which;
    if (event->charCode) return event->charCode;
    if (strlen(event->key) == 1) return (int)event->key[0];
    if (event->which) return event->which;
    return event->keyCode;
}*/

const char* WasmCanvas::OnBeforeUnloadCallback(int eventType, const void* /*reserved*/, void* userData)
{
	WasmCanvas* canvas = static_cast<WasmCanvas*>(userData);
    canvas->PostDestroy();
	return nullptr; // no string to be displayed to the user
}

void WasmCanvas::PostResizeFromDOMCanvas()
{
    /* Get canvas size from CSS layout (already reflects the current orientation) */
    double cssWidth = 0.0, cssHeight = 0.0;
    emscripten_get_element_css_size(canvasSelector_.c_str(), &cssWidth, &cssHeight);

    /* Convert CSS pixels to physical pixels for high-DPI displays */
    pixelRatio_ = std::min(emscripten_get_device_pixel_ratio(), maxPixelRatio_);
    const int w = static_cast<int>(std::round(cssWidth * pixelRatio_));
    const int h = static_cast<int>(std::round(cssHeight * pixelRatio_));

    if (w <= 0 || h <= 0 || (w == canvasSize_[0] && h == canvasSize_[1]))
        return;

    canvasSize_[0] = w;
    canvasSize_[1] = h;

    emscripten_set_canvas_element_size(canvasSelector_.c_str(), w, h);

    /* Send resize event to event listeners */
    const Extent2D clientAreaSize
    {
        static_cast<std::uint32_t>(w),
        static_cast<std::uint32_t>(h)
    };
    PostResize(clientAreaSize);
}

EM_BOOL WasmCanvas::OnOrientationChangeCallback(int eventType, const EmscriptenOrientationChangeEvent* event, void* userData)
{
    if (eventType == EMSCRIPTEN_EVENT_ORIENTATIONCHANGE)
    {
        /* Layout may not be updated yet at this point; the subsequent 'resize' event catches the final size */
        if (WasmCanvas* canvas = static_cast<WasmCanvas*>(userData))
            canvas->PostResizeFromDOMCanvas();
    }
    return EM_FALSE;
}

EM_BOOL WasmCanvas::OnCanvasResizeCallback(int eventType, const EmscriptenUiEvent* event, void* userData)
{
    if (eventType == EMSCRIPTEN_EVENT_RESIZE)
    {
        if (WasmCanvas* canvas = static_cast<WasmCanvas*>(userData))
            canvas->PostResizeFromDOMCanvas();
        return EM_TRUE;
    }
    return EM_FALSE;
}

EM_BOOL WasmCanvas::OnKeyCallback(int eventType, const EmscriptenKeyboardEvent* event, void* userData)
{
    WasmCanvas* canvas = static_cast<WasmCanvas*>(userData);

    Key key = MapEmscriptenKeyCode(event->code);

    EM_BOOL isEventHandled = EM_FALSE;

    if (eventType == 2)
    {
        if (canvas->PostKeyDown(key))
            isEventHandled = EM_TRUE;
    }
    else if (eventType == 3)
    {
        if (canvas->PostKeyUp(key))
            isEventHandled = EM_TRUE;
    }

    /* Don't consume function key events as they are reserved by the browser */
    if (key >= Key::F1 && key <= Key::F12)
        return EM_FALSE;

	return isEventHandled;
}

static Key EmscriptenMouseButtonToKeyCode(unsigned short button)
{
    switch (button)
    {
        case 0:     return Key::LButton;
        case 1:     return Key::MButton;
        case 2:     return Key::RButton;
        default:    return Key::Any;
    }
}

EM_BOOL WasmCanvas::OnMouseCallback(int eventType, const EmscriptenMouseEvent* event, void* userData)
{
    WasmCanvas* canvas = static_cast<WasmCanvas*>(userData);

    EM_BOOL isEventHandled = EM_FALSE;

    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_MOUSEDOWN:
        {
            if (canvas->PostKeyDown(EmscriptenMouseButtonToKeyCode(event->button)))
                isEventHandled = EM_TRUE;
        }
        break;

        case EMSCRIPTEN_EVENT_MOUSEUP:
        {
            if (canvas->PostKeyUp(EmscriptenMouseButtonToKeyCode(event->button)))
                isEventHandled = EM_TRUE;
        }
        break;

        case EMSCRIPTEN_EVENT_DBLCLICK:
        {
            if (canvas->PostDoubleClick(EmscriptenMouseButtonToKeyCode(event->button)))
                isEventHandled = EM_TRUE;
        }
        break;

        case EMSCRIPTEN_EVENT_MOUSEMOVE:
        {
            CanvasRect canvasRect;
            canvas->GetCanvasBoundingClientRect(canvasRect);
            const Offset2D position = canvas->ClientToCanvasPosition(canvasRect, event->clientX, event->clientY);
            const float motionX = static_cast<float>(event->movementX * canvas->pixelRatio_);
            const float motionY = static_cast<float>(event->movementY * canvas->pixelRatio_);
            if (canvas->PostMouseMotion(position, motionX, motionY))
                isEventHandled = EM_TRUE;
        }
        break;

        default:
        break;
    }

    return isEventHandled;
}

EM_BOOL WasmCanvas::OnWheelCallback(int eventType, const EmscriptenWheelEvent* event, void* userData)
{
    WasmCanvas* canvas = static_cast<WasmCanvas*>(userData);

    EM_BOOL isEventHandled = EM_FALSE;

    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_WHEEL:
        {
            const float motionX = static_cast<float>(event->deltaX);
            const float motionY = static_cast<float>(event->deltaY);
            if (canvas->PostWheelMotion(motionX, motionY))
                isEventHandled = EM_TRUE;
        }
        break;
    }

    return isEventHandled;
}

static EventAction EmscriptenTouchEventToAction(int eventType)
{
    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_TOUCHSTART:   return EventAction::Began;
        case EMSCRIPTEN_EVENT_TOUCHMOVE:    return EventAction::Changed;
        case EMSCRIPTEN_EVENT_TOUCHEND:     return EventAction::Ended;
        default:                            return EventAction::Ended; // fallback
    }
}

EM_BOOL WasmCanvas::OnTouchCallback(int eventType, const EmscriptenTouchEvent* event, void* userData)
{
    WasmCanvas* canvas = static_cast<WasmCanvas*>(userData);

    CanvasRect canvasRect;
    canvas->GetCanvasBoundingClientRect(canvasRect);

    const std::uint32_t numTouches = static_cast<std::uint32_t>(event->numTouches);

    auto TranslateTouchPosition = [canvas, &canvasRect, event](std::uint32_t touchIndex) -> Offset2D
    {
        return canvas->ClientToCanvasPosition(canvasRect, event->touches[touchIndex].clientX, event->touches[touchIndex].clientY);
    };

    EM_BOOL isEventHandled = EM_FALSE;

    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_TOUCHSTART:
        {
            for_range(touchIndex, numTouches)
            {
                const Offset2D position = TranslateTouchPosition(touchIndex);
                canvas->prevTouchPoint_[touchIndex] = position;
                if (canvas->PostPanGesture(position, touchIndex, numTouches, 0.0f, 0.0f, EventAction::Began))
                    isEventHandled = EM_TRUE;
            }
        }
        break;

        case EMSCRIPTEN_EVENT_TOUCHEND:
        case EMSCRIPTEN_EVENT_TOUCHMOVE:
        {
            const EventAction action = EmscriptenTouchEventToAction(eventType);
            for_range(touchIndex, numTouches)
            {
                const Offset2D position = TranslateTouchPosition(touchIndex);
                const float motionX = static_cast<float>(position.x - canvas->prevTouchPoint_[touchIndex].x);
                const float motionY = static_cast<float>(position.y - canvas->prevTouchPoint_[touchIndex].y);
                canvas->prevTouchPoint_[touchIndex] = position;
                if (canvas->PostPanGesture(position, touchIndex, numTouches, motionX, motionY, action))
                    isEventHandled = EM_TRUE;
            }
            if (eventType == EMSCRIPTEN_EVENT_TOUCHEND)
                isEventHandled = EM_FALSE; // Return false to indicate that the touchend event was not fully handled
        }
        break;

        default:
        break;
    }

    return isEventHandled;
}

void WasmCanvas::CreateEmscriptenCanvas(const CanvasDescriptor& desc)
{
    /* Find canvas handle*/
    emscripten::val config = emscripten::val::module_property("config");
    emscripten::val document = emscripten::val::global("document");
    
    canvasSelector_ = "#canvas";//config["canvas_selector"].as<std::string>();
    canvas_ = document["body"].call<emscripten::val>("querySelector", canvasSelector_);

    /* Set title and show canvas (if enabled) */
    SetTitle(desc.title);

    /* Set callbacks */
    EMSCRIPTEN_RESULT ret;
    ret = emscripten_set_beforeunload_callback(this, WasmCanvas::OnBeforeUnloadCallback);
    ret = emscripten_set_orientationchange_callback(this, EM_TRUE, WasmCanvas::OnOrientationChangeCallback);
    ret = emscripten_set_resize_callback    (EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnCanvasResizeCallback);
	ret = emscripten_set_keydown_callback   (/*canvasSelector_.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnKeyCallback);
	ret = emscripten_set_keyup_callback     (/*canvasSelector_.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnKeyCallback);
    ret = emscripten_set_click_callback     (/*canvasSelector_.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnMouseCallback);
    ret = emscripten_set_dblclick_callback  (/*canvasSelector_.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnMouseCallback);
    ret = emscripten_set_mousedown_callback (/*canvasSelector_.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnMouseCallback);
    ret = emscripten_set_mouseup_callback   (/*canvasSelector_.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnMouseCallback);
    ret = emscripten_set_mousemove_callback (/*canvasSelector_.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnMouseCallback);
    ret = emscripten_set_mouseenter_callback(/*canvasSelector_.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnMouseCallback);
    ret = emscripten_set_mouseleave_callback(/*canvasSelector_.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnMouseCallback);
    ret = emscripten_set_wheel_callback     (/*canvasSelector_.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnWheelCallback);
    ret = emscripten_set_touchstart_callback(/*canvasSelector_.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnTouchCallback);
    ret = emscripten_set_touchend_callback  (/*canvasSelector_.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnTouchCallback);
    ret = emscripten_set_touchmove_callback (/*canvasSelector_.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnTouchCallback);
    ret = emscripten_set_touchcancel_callback(/*canvasSelector_.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnTouchCallback);

    /* Resize canvas to initial CSS size */
    PostResizeFromDOMCanvas();
}

void WasmCanvas::GetCanvasBoundingClientRect(CanvasRect& outRect) const
{
    EM_ASM(
        {
            var id = UTF8ToString($0);
            var canvas = id ? document.querySelector(id) : Module['canvas'];
            if (canvas) {
                var r = canvas.getBoundingClientRect();
                // Write directly back to C struct memory pointers
                setValue($1, r.left,    'double');
                setValue($2, r.top,     'double');
                setValue($3, r.width,   'double');
                setValue($4, r.height,  'double');
            }
        },
        canvasSelector_.c_str(), &outRect.x, &outRect.y, &outRect.width, &outRect.height
    );
}

Offset2D WasmCanvas::ClientToCanvasPosition(const CanvasRect& canvasRect, double clientX, double clientY) const
{
    return Offset2D
    {
        static_cast<std::int32_t>((clientX - canvasRect.x) * pixelRatio_),
        static_cast<std::int32_t>((clientY - canvasRect.y) * pixelRatio_)
    };
}


} // /namespace LLGL



// ================================================================================
