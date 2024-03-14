/*
 * EmscriptenWindow.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Display.h>
#include "EmscriptenWindow.h"
#include "MapKey.h"
#include "../../Core/CoreUtils.h"
#include <exception>
#include <emscripten/key_codes.h>

namespace LLGL
{


/*
 * Surface class
 */

bool Surface::ProcessEvents()
{
 

    return true;
}


/*
 * Window class
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

std::unique_ptr<Window> Window::Create(const WindowDescriptor& desc)
{
    return MakeUnique<EmscriptenWindow>(desc);
}


/*
 * EmscriptenWindow class
 */

EmscriptenWindow::EmscriptenWindow(const WindowDescriptor& desc) :
    desc_ { desc }
{
    CreateEmscriptenWindow();
}

EmscriptenWindow::~EmscriptenWindow()
{
    
}

bool EmscriptenWindow::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(NativeHandle))
    {
        auto* handle = reinterpret_cast<NativeHandle*>(nativeHandle);
        //handle->visual  = visual_;
        return true;
    }
    return false;
}

void EmscriptenWindow::ResetPixelFormat()
{
    // dummy
}

Extent2D EmscriptenWindow::GetContentSize() const
{
    /* Return the size of the client area */
    return GetSize(true);
}

void EmscriptenWindow::SetPosition(const Offset2D& position)
{
    /* Move window and store new position */
    //XMoveWindow(display_, wnd_, position.x, position.y);
    //desc_.position = position;
}

Offset2D EmscriptenWindow::GetPosition() const
{
    //XWindowAttributes attribs;
    //XGetWindowAttributes(display_, wnd_, &attribs);
    //return { attribs.x, attribs.y };

    return {0, 0};
}

void EmscriptenWindow::SetSize(const Extent2D& size, bool useClientArea)
{
    //XResizeWindow(display_, wnd_, size.width, size.height);
}

Extent2D EmscriptenWindow::GetSize(bool useClientArea) const
{
    /*
    XWindowAttributes attribs;
    XGetWindowAttributes(display_, wnd_, &attribs);
    */

    return Extent2D
    {
        static_cast<std::uint32_t>(0),
        static_cast<std::uint32_t>(0)
    };
}

void EmscriptenWindow::SetTitle(const UTF8String& title)
{
    //XStoreName(display_, wnd_, title.c_str());
}

UTF8String EmscriptenWindow::GetTitle() const
{
    char* title = nullptr;
    //XFetchName(display_, wnd_, &title);
    return title;
}

void EmscriptenWindow::Show(bool show)
{
    if (show)
    {
        /* Map window and reset window position */
        //XMapWindow(display_, wnd_);
        //XMoveWindow(display_, wnd_, desc_.position.x, desc_.position.y);
    }
    else
    {
        //XUnmapWindow(display_, wnd_);
    }

    if ((desc_.flags & WindowFlags::Borderless) != 0)
    {
        //XSetInputFocus(display_, (show ? wnd_ : None), RevertToParent, CurrentTime);
    }
}

bool EmscriptenWindow::IsShown() const
{
    return false;
}

void EmscriptenWindow::SetDesc(const WindowDescriptor& desc)
{
    //todo...
}

WindowDescriptor EmscriptenWindow::GetDesc() const
{
    return desc_; //todo...
}

void EmscriptenWindow::ProcessEvent(/*XEvent& event*/)
{
    /*
    switch (event.type)
    {
        case KeyPress:
            ProcessKeyEvent(event.xkey, true);
            break;

        case KeyRelease:
            ProcessKeyEvent(event.xkey, false);
            break;

        case ButtonPress:
            ProcessMouseKeyEvent(event.xbutton, true);
            break;

        case ButtonRelease:
            ProcessMouseKeyEvent(event.xbutton, false);
            break;

        case Expose:
            ProcessExposeEvent();
            break;

        case MotionNotify:
            ProcessMotionEvent(event.xmotion);
            break;

        case DestroyNotify:
            PostQuit();
            break;

        case ClientMessage:
            ProcessClientMessage(event.xclient);
            break;
    }
    */
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

static inline const char *emscripten_event_type_to_string(int eventType)
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

const char *emscripten_result_to_string(EMSCRIPTEN_RESULT result)
{
    if (result == EMSCRIPTEN_RESULT_SUCCESS) return "EMSCRIPTEN_RESULT_SUCCESS";
    if (result == EMSCRIPTEN_RESULT_DEFERRED) return "EMSCRIPTEN_RESULT_DEFERRED";
    if (result == EMSCRIPTEN_RESULT_NOT_SUPPORTED) return "EMSCRIPTEN_RESULT_NOT_SUPPORTED";
    if (result == EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED) return "EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED";
    if (result == EMSCRIPTEN_RESULT_INVALID_TARGET) return "EMSCRIPTEN_RESULT_INVALID_TARGET";
    if (result == EMSCRIPTEN_RESULT_UNKNOWN_TARGET) return "EMSCRIPTEN_RESULT_UNKNOWN_TARGET";
    if (result == EMSCRIPTEN_RESULT_INVALID_PARAM) return "EMSCRIPTEN_RESULT_INVALID_PARAM";
    if (result == EMSCRIPTEN_RESULT_FAILED) return "EMSCRIPTEN_RESULT_FAILED";
    if (result == EMSCRIPTEN_RESULT_NO_DATA) return "EMSCRIPTEN_RESULT_NO_DATA";
    return "Unknown EMSCRIPTEN_RESULT!";
}

int interpret_charcode_for_keyevent(int eventType, const EmscriptenKeyboardEvent *e)
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

const char* EmscriptenWindow::OnBeforeUnloadCallback(int eventType, const void* reserved, void* userData)
{
	EmscriptenWindow *emscriptenWindow = ((EmscriptenWindow*)userData);
	//cleanup the window
	return NULL;
}

int EmscriptenWindow::OnCanvasResizeCallback(int eventType, const EmscriptenUiEvent *keyEvent, void *userData)
{
    EmscriptenWindow* window = (EmscriptenWindow*)userData;

	unsigned int width, height;
	width = keyEvent->windowInnerWidth;
	height = keyEvent->windowInnerHeight;
    //EM_ASM({console.log("OnCanvasResizeCallback");});
    window->PostResize(Extent2D{ width, height });
	return 0;
}

EM_BOOL EmscriptenWindow::OnKeyCallback(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
    EmscriptenWindow* window = (EmscriptenWindow*)userData;

    int dom_pk_code = emscripten_compute_dom_pk_code(e->code);

    /*
    printf("%s, key: \"%s\", code: \"%s\" = %s (%d), location: %lu,%s%s%s%s repeat: %d, locale: \"%s\", char: \"%s\", charCode: %lu (interpreted: %d), keyCode: %s(%lu), which: %lu\n",
        emscripten_event_type_to_string(eventType), e->key, e->code, emscripten_dom_pk_code_to_string(dom_pk_code), dom_pk_code, e->location,
        e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "", 
        e->repeat, e->locale, e->charValue, e->charCode, interpret_charcode_for_keyevent(eventType, e), emscripten_dom_vk_to_string(e->keyCode), e->keyCode, e->which);

    if (eventType == EMSCRIPTEN_EVENT_KEYUP) printf("\n"); // Visual cue
    */

    printf("%s, key: \"%s\", code: \"%s\" = %s (%d)\n", emscripten_event_type_to_string(eventType), e->key, e->code, emscripten_dom_pk_code_to_string(dom_pk_code), dom_pk_code);
    EmscriptenWindow *emscriptenWindow = ((EmscriptenWindow*)userData);
 
    auto key = MapKey(e->code);

    if (eventType == 2)
        window->PostKeyDown(key);
    else if (eventType == 3)
        window->PostKeyUp(key);

	return true;
}

EM_BOOL EmscriptenWindow::OnMouseCallback(int eventType, const EmscriptenMouseEvent *e, void *userData)
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

EM_BOOL EmscriptenWindow::OnWheelCallback(int eventType, const EmscriptenWheelEvent *e, void *userData)
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

void EmscriptenWindow::CreateEmscriptenWindow()
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

    //EM_ASM({console.log("isUndefined");});

    EM_ASM({
        console.log(Emval.toValue($0));
    }, canvas_.as_handle());

    //emscripten::val console = emscripten::val::global("console");
    //console.call<void>("log", canvas);

    /* Get final window position */
    if ((desc_.flags & WindowFlags::Centered) != 0)
        desc_.position = GetScreenCenteredPosition(desc_.size);

    /* Set title and show window (if enabled) */
    SetTitle(desc_.title);

    /* Show window */
    if ((desc_.flags & WindowFlags::Visible) != 0)
    {

    }

    /* Prepare borderless window */
    const bool isBorderless = ((desc_.flags & WindowFlags::Borderless) != 0);
    if (isBorderless)
    {
    }

    /* Set callbacks */
    EMSCRIPTEN_RESULT ret;
    ret = emscripten_set_beforeunload_callback((void*)this, EmscriptenWindow::OnBeforeUnloadCallback);
    ret = emscripten_set_resize_callback(canvas_selector.c_str(), (void*)this, true, EmscriptenWindow::OnCanvasResizeCallback);

	ret = emscripten_set_keydown_callback(canvas_selector.c_str(), (void*)this, true, EmscriptenWindow::OnKeyCallback);
	ret = emscripten_set_keyup_callback(canvas_selector.c_str(), (void*)this, true, EmscriptenWindow::OnKeyCallback);

    ret = emscripten_set_click_callback(canvas_selector.c_str(), (void*)this, true, EmscriptenWindow::OnMouseCallback);
    ret = emscripten_set_mousedown_callback(canvas_selector.c_str(), (void*)this, true, EmscriptenWindow::OnMouseCallback);
    ret = emscripten_set_mouseup_callback(canvas_selector.c_str(), (void*)this, true, EmscriptenWindow::OnMouseCallback);
    ret = emscripten_set_dblclick_callback(canvas_selector.c_str(), (void*)this, true, EmscriptenWindow::OnMouseCallback);
    ret = emscripten_set_mousemove_callback(canvas_selector.c_str(), (void*)this, true, EmscriptenWindow::OnMouseCallback);
    ret = emscripten_set_wheel_callback(canvas_selector.c_str(), (void*)this, true, EmscriptenWindow::OnWheelCallback);
}

void EmscriptenWindow::ProcessKeyEvent(/*XKeyEvent& event, bool down*/)
{
    /*auto key = MapKey(event);
    if (down)
        PostKeyDown(key);
    else
        PostKeyUp(key);*/
}

void EmscriptenWindow::ProcessMouseKeyEvent(/*XButtonEvent& event, bool down*/)
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

void EmscriptenWindow::ProcessExposeEvent()
{
    //XWindowAttributes attribs;
    //XGetWindowAttributes(display_, wnd_, &attribs);

    const Extent2D size
    {
        static_cast<std::uint32_t>(0),
        static_cast<std::uint32_t>(0)
    };

    PostResize(size);
}

void EmscriptenWindow::ProcessClientMessage(/*XClientMessageEvent& event*/)
{
    /*Atom atom = static_cast<Atom>(event.data.l[0]);
    if (atom == closeWndAtom_)
        PostQuit();*/
}

void EmscriptenWindow::ProcessMotionEvent(/*XMotionEvent& event*/)
{
    /*const Offset2D mousePos { event.x, event.y };
    PostLocalMotion(mousePos);
    PostGlobalMotion({ mousePos.x - prevMousePos_.x, mousePos.y - prevMousePos_.y });
    prevMousePos_ = mousePos;*/
}

void EmscriptenWindow::PostMouseKeyEvent(Key key, bool down)
{
    if (down)
        PostKeyDown(key);
    else
        PostKeyUp(key);
}


} // /namespace LLGL



// ================================================================================
