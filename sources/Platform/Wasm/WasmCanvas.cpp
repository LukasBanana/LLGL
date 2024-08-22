/*
 * EmscriptenCanvas.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "EmscriptenCanvas.h"
#include "MapKey.h"
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

static Offset2D GetScreenCenteredPosition(const Extent2D& size)
{
    if (auto display = Display::GetPrimary())
    {
        const auto resolution = display->GetDisplayMode().resolution;
        return
        {
            static_cast<int>((resolution.width  - size.width )/2),
            static_cast<int>((resolution.height - size.height)/2),
        };
    }
    return {};
}

std::unique_ptr<Canvas> Canvas::Create(const CanvasDescriptor& desc)
{
    return MakeUnique<EmscriptenCanvas>(desc);
}


/*
 * EmscriptenCanvas class
 */

EmscriptenCanvas::EmscriptenCanvas(const CanvasDescriptor& desc)
{
    CreateEmscriptenCanvas(desc);
}

bool EmscriptenCanvas::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(NativeHandle))
    {
        auto* handle = reinterpret_cast<NativeHandle*>(nativeHandle);
        //handle->visual  = visual_;
        return true;
    }
    return false;
}

Extent2D EmscriptenCanvas::GetContentSize() const
{
    int width = 0, height = 0;
    emscripten_get_canvas_element_size("#canvas", &width, &height);
    return Extent2D
    {
        static_cast<std::uint32_t>(width),
        static_cast<std::uint32_t>(height)
    };
}

void EmscriptenCanvas::SetTitle(const UTF8String& title)
{
}

UTF8String EmscriptenCanvas::GetTitle() const
{
    char* title = nullptr;
    return title;
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

static int interpret_charcode_for_keyevent(int eventType, const EmscriptenKeyboardEvent *e)
{
    // Only KeyPress events carry a charCode. For KeyDown and KeyUp events, these don't seem to be present yet, until later when the KeyDown
    // is transformed to KeyPress. Sometimes it can be useful to already at KeyDown time to know what the charCode of the resulting
    // KeyPress will be. The following attempts to do this:
    if (eventType == EMSCRIPTEN_EVENT_KEYPRESS && e->which) return e->which;
    if (e->charCode) return e->charCode;
    if (strlen(e->key) == 1) return (int)e->key[0];
    if (e->which) return e->which;
    return e->keyCode;
}

const char* EmscriptenCanvas::OnBeforeUnloadCallback(int eventType, const void* reserved, void* userData)
{
	EmscriptenCanvas* canvas = reinterpret_cast<EmscriptenCanvas*>(userData);
    canvas->PostDestroy();
	return nullptr;
}

int EmscriptenCanvas::OnCanvasResizeCallback(int eventType, const EmscriptenUiEvent* event, void *userData)
{
    EmscriptenCanvas* canvas = reinterpret_cast<EmscriptenCanvas*>(userData);
    const Extent2D clientAreaSize
    {
        static_cast<std::uint32_t>(event->documentBodyClientWidth),
        static_cast<std::uint32_t>(event->documentBodyClientHeight)
    };
    canvas->PostResize(clientAreaSize);
	return 0;
}

EM_BOOL EmscriptenCanvas::OnKeyCallback(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
    EmscriptenCanvas* canvas = reinterpret_cast<EmscriptenCanvas*>(userData);

    int dom_pk_code = emscripten_compute_dom_pk_code(e->code);

    /*
    printf("%s, key: \"%s\", code: \"%s\" = %s (%d), location: %lu,%s%s%s%s repeat: %d, locale: \"%s\", char: \"%s\", charCode: %lu (interpreted: %d), keyCode: %s(%lu), which: %lu\n",
        emscripten_event_type_to_string(eventType), e->key, e->code, emscripten_dom_pk_code_to_string(dom_pk_code), dom_pk_code, e->location,
        e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "", 
        e->repeat, e->locale, e->charValue, e->charCode, interpret_charcode_for_keyevent(eventType, e), emscripten_dom_vk_to_string(e->keyCode), e->keyCode, e->which);

    if (eventType == EMSCRIPTEN_EVENT_KEYUP) printf("\n"); // Visual cue
    */

    printf("%s, key: \"%s\", code: \"%s\" = %s (%d)\n", emscripten_event_type_to_string(eventType), e->key, e->code, emscripten_dom_pk_code_to_string(dom_pk_code), dom_pk_code);
 
    auto key = MapKey(e->code);

    if (eventType == 2)
        canvas->PostKeyDown(key);
    else if (eventType == 3)
        canvas->PostKeyUp(key);

	return true;
}

EM_BOOL EmscriptenCanvas::OnMouseCallback(int eventType, const EmscriptenMouseEvent *e, void *userData)
{
    /*
    printf("%s, screen: (%ld,%ld), client: (%ld,%ld),%s%s%s%s button: %hu, buttons: %hu, movement: (%ld,%ld), canvas: (%ld,%ld), target: (%ld, %ld)\n",
        emscripten_event_type_to_string(eventType), e->screenX, e->screenY, e->clientX, e->clientY,
        e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "", 
        e->button, e->buttons, e->movementX, e->movementY, e->canvasX, e->canvasY, e->targetX, e->targetY);
    */

    /*
    if (e->screenX != 0 && e->screenY != 0 && e->clientX != 0 && e->clientY != 0 && e->canvasX != 0 && e->canvasY != 0 && e->targetX != 0 && e->targetY != 0)
    {
        if (eventType == EMSCRIPTEN_EVENT_CLICK) gotClick = 1;
        if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN && e->buttons != 0) gotMouseDown = 1;
        if (eventType == EMSCRIPTEN_EVENT_DBLCLICK) gotDblClick = 1;
        if (eventType == EMSCRIPTEN_EVENT_MOUSEUP) gotMouseUp = 1;
        if (eventType == EMSCRIPTEN_EVENT_MOUSEMOVE && (e->movementX != 0 || e->movementY != 0)) gotMouseMove = 1;
    }
    */

    /*
    if (eventType == EMSCRIPTEN_EVENT_CLICK && e->screenX == -500000)
    {
        printf("ERROR! Received an event to a callback that should have been unregistered!\n");
        gotClick = 0;
    }
    */

    return EMSCRIPTEN_RESULT_SUCCESS;
}

EM_BOOL EmscriptenCanvas::OnWheelCallback(int eventType, const EmscriptenWheelEvent *e, void *userData)
{
    /*
    printf("%s, screen: (%ld,%ld), client: (%ld,%ld),%s%s%s%s button: %hu, buttons: %hu, canvas: (%ld,%ld), target: (%ld, %ld), delta:(%g,%g,%g), deltaMode:%lu\n",
        emscripten_event_type_to_string(eventType), e->mouse.screenX, e->mouse.screenY, e->mouse.clientX, e->mouse.clientY,
        e->mouse.ctrlKey ? " CTRL" : "", e->mouse.shiftKey ? " SHIFT" : "", e->mouse.altKey ? " ALT" : "", e->mouse.metaKey ? " META" : "", 
        e->mouse.button, e->mouse.buttons, e->mouse.canvasX, e->mouse.canvasY, e->mouse.targetX, e->mouse.targetY,
        (float)e->deltaX, (float)e->deltaY, (float)e->deltaZ, e->deltaMode);
    */

    /*
    if (e->deltaY > 0.f || e->deltaY < 0.f)
        gotWheel = 1;
    */

    return EMSCRIPTEN_RESULT_SUCCESS;
}

void EmscriptenCanvas::CreateEmscriptenCanvas(const CanvasDescriptor& desc)
{
    /* Find canvas handle*/
    emscripten::val config = emscripten::val::module_property("config");
    emscripten::val document = emscripten::val::global("document");
    
    if (config.isUndefined() || config.isNull())
        return;
    

    if (!config.hasOwnProperty("canvas_selector"))
        return;
    
    std::string canvas_selector = config["canvas_selector"].as<std::string>();
    canvas_ = document["body"].call<emscripten::val>("querySelector", canvas_selector);

    EM_ASM({
        console.log(Emval.toValue($0));
    }, canvas_.as_handle());

    //emscripten::val console = emscripten::val::global("console");
    //console.call<void>("log", canvas);

    /* Set title and show canvas (if enabled) */
    SetTitle(desc.title);

    /* Set callbacks */
    EMSCRIPTEN_RESULT ret;
    ret = emscripten_set_beforeunload_callback(this, EmscriptenCanvas::OnBeforeUnloadCallback);
    ret = emscripten_set_resize_callback(canvas_selector.c_str(), this, true, EmscriptenCanvas::OnCanvasResizeCallback);

	ret = emscripten_set_keydown_callback(canvas_selector.c_str(), this, true, EmscriptenCanvas::OnKeyCallback);
	ret = emscripten_set_keyup_callback(canvas_selector.c_str(), this, true, EmscriptenCanvas::OnKeyCallback);

    ret = emscripten_set_click_callback(canvas_selector.c_str(), this, true, EmscriptenCanvas::OnMouseCallback);
    ret = emscripten_set_mousedown_callback(canvas_selector.c_str(), this, true, EmscriptenCanvas::OnMouseCallback);
    ret = emscripten_set_mouseup_callback(canvas_selector.c_str(), this, true, EmscriptenCanvas::OnMouseCallback);
    ret = emscripten_set_dblclick_callback(canvas_selector.c_str(), this, true, EmscriptenCanvas::OnMouseCallback);
    ret = emscripten_set_mousemove_callback(canvas_selector.c_str(), this, true, EmscriptenCanvas::OnMouseCallback);
    ret = emscripten_set_wheel_callback(canvas_selector.c_str(), this, true, EmscriptenCanvas::OnWheelCallback);
}

void EmscriptenCanvas::ProcessKeyEvent(/*event, bool down*/)
{
    /*auto key = MapKey(event);
    if (down)
        PostKeyDown(key);
    else
        PostKeyUp(key);*/
}

void EmscriptenCanvas::ProcessMouseKeyEvent(/*event, bool down*/)
{
    /*switch (event.button)
    {
        case Button1:
            PostMouseKeyEvent(Key::LButton, down);
            break;
        case Button2:
            PostMouseKeyEvent(Key::MButton, down);
            break;
        case Button3:
            PostMouseKeyEvent(Key::RButton, down);
            break;
        case Button4:
            PostWheelMotion(1);
            break;
        case Button5:
            PostWheelMotion(-1);
            break;
    }*/
}

/*void EmscriptenCanvas::ProcessExposeEvent()
{
    const Extent2D size
    {
        static_cast<std::uint32_t>(0),
        static_cast<std::uint32_t>(0)
    };
    PostResize(size);
}*/

void EmscriptenCanvas::ProcessClientMessage(/*XClientMessageEvent& event*/)
{
    /*Atom atom = static_cast<Atom>(event.data.l[0]);
    if (atom == closeWndAtom_)
        PostQuit();*/
}

void EmscriptenCanvas::ProcessMotionEvent(/*XMotionEvent& event*/)
{
    /*const Offset2D mousePos { event.x, event.y };
    PostLocalMotion(mousePos);
    PostGlobalMotion({ mousePos.x - prevMousePos_.x, mousePos.y - prevMousePos_.y });
    prevMousePos_ = mousePos;*/
}

void EmscriptenCanvas::PostMouseKeyEvent(Key key, bool down)
{
    if (down)
        PostKeyDown(key);
    else
        PostKeyUp(key);
}


} // /namespace LLGL



// ================================================================================
