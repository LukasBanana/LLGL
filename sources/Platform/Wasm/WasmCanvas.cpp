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
#include "../../Core/CoreUtils.h"
#include <exception>


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
        auto* handle = reinterpret_cast<NativeHandle*>(nativeHandle);
        handle->canvas = canvas_;
        return true;
    }
    return false;
}

Extent2D WasmCanvas::GetContentSize() const
{
    int width = 0, height = 0;
    emscripten_get_canvas_element_size("#canvas", &width, &height);
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

/*
EM_JS(emscripten::EM_VAL, get_canvas, (), {
    let canvas = document.getElementById("mycanvas");
    return Emval.toHandle(canvas);
});
canvas_ = emscripten::val::take_ownership(get_canvas());
*/

static const char* emscripten_event_type_to_string(int eventType)
{
    const char *events[] = { "(invalid)", "(none)", "keypress", "keydown", "keyup", "click", "mousedown", "mouseup", "dblclick", "mousemove", "wheel", "resize",
    "scroll", "blur", "focus", "focusin", "focusout", "deviceorientation", "devicemotion", "orientationchange", "fullscreenchange", "pointerlockchange",
    "visibilitychange", "touchstart", "touchend", "touchmove", "touchcancel", "gamepadconnected", "gamepaddisconnected", "beforeunload",
    "batterychargingchange", "batterylevelchange", "webglcontextlost", "webglcontextrestored", "(invalid)" };

    ++eventType;

    if (eventType < 0)
        eventType = 0;

    if (eventType >= sizeof(events)/sizeof(events[0]))
        eventType = sizeof(events)/sizeof(events[0])-1;

    return events[eventType];
}

static const char* EmscriptenResultToString(EMSCRIPTEN_RESULT result)
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
}

static int interpret_charcode_for_keyevent(int eventType, const EmscriptenKeyboardEvent* event)
{
    // Only KeyPress events carry a charCode. For KeyDown and KeyUp events, these don't seem to be present yet, until later when the KeyDown
    // is transformed to KeyPress. Sometimes it can be useful to already at KeyDown time to know what the charCode of the resulting
    // KeyPress will be. The following attempts to do this:
    if (eventType == EMSCRIPTEN_EVENT_KEYPRESS && event->which) return event->which;
    if (event->charCode) return event->charCode;
    if (strlen(event->key) == 1) return (int)event->key[0];
    if (event->which) return event->which;
    return event->keyCode;
}

const char* WasmCanvas::OnBeforeUnloadCallback(int eventType, const void* reserved, void* userData)
{
	WasmCanvas* canvas = reinterpret_cast<WasmCanvas*>(userData);
    canvas->PostDestroy();
	return nullptr;
}

EM_BOOL WasmCanvas::OnCanvasResizeCallback(int eventType, const EmscriptenUiEvent* event, void* userData)
{
    if (eventType == EMSCRIPTEN_EVENT_RESIZE)
    {
        /* Resize this canvas as the event comes from the window and we must update the canvas resolution */
        emscripten_set_canvas_element_size("#canvas", event->windowInnerWidth, event->windowInnerHeight);

        /* Send resize event to event listeners */
        WasmCanvas* canvas = reinterpret_cast<WasmCanvas*>(userData);
        const Extent2D clientAreaSize
        {
            static_cast<std::uint32_t>(event->windowInnerWidth),
            static_cast<std::uint32_t>(event->windowInnerHeight)
        };
        canvas->PostResize(clientAreaSize);
        return EM_TRUE;
    }
    return EM_FALSE;
}

EM_BOOL WasmCanvas::OnKeyCallback(int eventType, const EmscriptenKeyboardEvent* event, void* userData)
{
    WasmCanvas* canvas = reinterpret_cast<WasmCanvas*>(userData);

    int dom_pk_code = emscripten_compute_dom_pk_code(event->code);

    /*
    printf("%s, key: \"%s\", code: \"%s\" = %s (%d), location: %lu,%s%s%s%s repeat: %d, locale: \"%s\", char: \"%s\", charCode: %lu (interpreted: %d), keyCode: %s(%lu), which: %lu\n",
        emscripten_event_type_to_string(eventType), event->key, event->code, emscripten_dom_pk_code_to_string(dom_pk_code), dom_pk_code, event->location,
        event->ctrlKey ? " CTRL" : "", event->shiftKey ? " SHIFT" : "", event->altKey ? " ALT" : "", event->metaKey ? " META" : "", 
        event->repeat, event->locale, event->charValue, event->charCode, interpret_charcode_for_keyevent(eventType, e), emscripten_dom_vk_to_string(event->keyCode), event->keyCode, event->which);

    if (eventType == EMSCRIPTEN_EVENT_KEYUP) printf("\n"); // Visual cue
    */

    printf("%s, key: \"%s\", code: \"%s\" = %s (%d)\n", emscripten_event_type_to_string(eventType), event->key, event->code, emscripten_dom_pk_code_to_string(dom_pk_code), dom_pk_code);
 
    auto key = MapEmscriptenKeyCode(event->code);

    if (eventType == 2)
        canvas->PostKeyDown(key);
    else if (eventType == 3)
        canvas->PostKeyUp(key);

	return EM_TRUE;
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

static EventAction EmscriptenMouseEventToAction(int eventType)
{
    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_MOUSEENTER:   return EventAction::Began;
        case EMSCRIPTEN_EVENT_MOUSEMOVE:    return EventAction::Changed;
        case EMSCRIPTEN_EVENT_MOUSELEAVE:   return EventAction::Ended;
        default:                            return EventAction::Ended; // fallback
    }
}

EM_BOOL WasmCanvas::OnMouseCallback(int eventType, const EmscriptenMouseEvent* event, void* userData)
{
    WasmCanvas* canvas = reinterpret_cast<WasmCanvas*>(userData);

    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_MOUSEDOWN:
        {
            canvas->PostKeyDown(EmscriptenMouseButtonToKeyCode(event->button));
        }
        return EM_TRUE;

        case EMSCRIPTEN_EVENT_MOUSEUP:
        {
            canvas->PostKeyUp(EmscriptenMouseButtonToKeyCode(event->button));
        }
        return EM_TRUE;

        case EMSCRIPTEN_EVENT_CLICK:
        case EMSCRIPTEN_EVENT_DBLCLICK:
        {
            const Offset2D position
            {
                static_cast<std::int32_t>(event->clientX),
                static_cast<std::int32_t>(event->clientY)
            };
            canvas->PostTapGesture(position, 1);
        }
        return EM_TRUE;

        case EMSCRIPTEN_EVENT_MOUSEENTER:
        case EMSCRIPTEN_EVENT_MOUSEMOVE:
        case EMSCRIPTEN_EVENT_MOUSELEAVE:
        {
            const Offset2D position
            {
                static_cast<std::int32_t>(event->clientX),
                static_cast<std::int32_t>(event->clientY)
            };
            const float motionX = static_cast<float>(event->movementX);
            const float motionY = static_cast<float>(event->movementY);
            canvas->PostPanGesture(position, 1, motionX, motionY, EmscriptenMouseEventToAction(eventType));
        }
        return EM_TRUE;

        default:
        break;
    }

    return EM_FALSE;
}

EM_BOOL WasmCanvas::OnWheelCallback(int eventType, const EmscriptenWheelEvent* event, void* userData)
{
    /*
    printf("%s, screen: (%ld,%ld), client: (%ld,%ld),%s%s%s%s button: %hu, buttons: %hu, canvas: (%ld,%ld), target: (%ld, %ld), delta:(%g,%g,%g), deltaMode:%lu\n",
        emscripten_event_type_to_string(eventType), event->mouse.screenX, event->mouse.screenY, event->mouse.clientX, event->mouse.clientY,
        event->mouse.ctrlKey ? " CTRL" : "", event->mouse.shiftKey ? " SHIFT" : "", event->mouse.altKey ? " ALT" : "", event->mouse.metaKey ? " META" : "", 
        event->mouse.button, event->mouse.buttons, event->mouse.canvasX, event->mouse.canvasY, event->mouse.targetX, event->mouse.targetY,
        (float)event->deltaX, (float)event->deltaY, (float)event->deltaZ, event->deltaMode);
    */

    /*
    if (event->deltaY > 0.f || event->deltaY < 0.f)
        gotWheel = 1;
    */

    return EM_TRUE;
}

void WasmCanvas::CreateEmscriptenCanvas(const CanvasDescriptor& desc)
{
    /* Find canvas handle*/
    emscripten::val config = emscripten::val::module_property("config");
    emscripten::val document = emscripten::val::global("document");
    
    //if (config.isUndefined() || config.isNull())
    //    return;
    
    //if (!config.hasOwnProperty("canvas_selector"))
    //    return;
    
    std::string canvasSelector = "#canvas";//config["canvas_selector"].as<std::string>();
    canvas_ = document["body"].call<emscripten::val>("querySelector", canvasSelector);

    EM_ASM({
        console.log(Emval.toValue($0));
    }, canvas_.as_handle());

    //emscripten::val console = emscripten::val::global("console");
    //console.call<void>("log", canvas);

    /* Set title and show canvas (if enabled) */
    SetTitle(desc.title);

    /* Set callbacks */
    EMSCRIPTEN_RESULT ret;
    ret = emscripten_set_beforeunload_callback(this, WasmCanvas::OnBeforeUnloadCallback);
    ret = emscripten_set_resize_callback    (EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnCanvasResizeCallback);
	ret = emscripten_set_keydown_callback   (/*canvasSelector.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnKeyCallback);
	ret = emscripten_set_keyup_callback     (/*canvasSelector.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnKeyCallback);
    ret = emscripten_set_click_callback     (/*canvasSelector.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnMouseCallback);
    ret = emscripten_set_dblclick_callback  (/*canvasSelector.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnMouseCallback);
    ret = emscripten_set_mousedown_callback (/*canvasSelector.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnMouseCallback);
    ret = emscripten_set_mouseup_callback   (/*canvasSelector.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnMouseCallback);
    ret = emscripten_set_mousemove_callback (/*canvasSelector.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnMouseCallback);
    ret = emscripten_set_mouseenter_callback(/*canvasSelector.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnMouseCallback);
    ret = emscripten_set_mouseleave_callback(/*canvasSelector.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnMouseCallback);
    ret = emscripten_set_wheel_callback     (/*canvasSelector.c_str()*/EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, WasmCanvas::OnWheelCallback);
}


} // /namespace LLGL



// ================================================================================
