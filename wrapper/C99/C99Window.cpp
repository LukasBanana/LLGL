/*
 * C99Window.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Window.h>
#include <LLGL-C/Window.h>
#include "C99Internal.h"
#include "C99EventListenerContainer.h"
#include "../sources/Core/CoreUtils.h"
#include "../sources/Core/Exception.h"
#include <vector>
#include <string>
#include <string.h>
#include <algorithm>


// namespace LLGL {


using namespace LLGL;

#define LLGL_CALLBACK_WRAPPER(FUNC, ...) \
    if (callbacks_.FUNC != NULL) { callbacks_.FUNC(LLGLWindow{ &sender } LLGL_VA_ARGS(__VA_ARGS__)); }

class InternalWindowEventListener final : public Window::EventListener
{

        LLGLWindowEventListener callbacks_;

    public:

        InternalWindowEventListener(const LLGLWindowEventListener* callbacks)
        {
            memcpy(&callbacks_, callbacks, sizeof(LLGLWindowEventListener));
        }

        void OnQuit(Window& sender, bool& veto) override
        {
            LLGL_CALLBACK_WRAPPER(onQuit, &veto);
        }

        void OnKeyDown(Window& sender, Key keyCode) override
        {
            LLGL_CALLBACK_WRAPPER(onKeyDown, (LLGLKey)keyCode);
        }

        void OnKeyUp(Window& sender, Key keyCode) override
        {
            LLGL_CALLBACK_WRAPPER(onKeyUp, (LLGLKey)keyCode);
        }

        void OnDoubleClick(Window& sender, Key keyCode) override
        {
            LLGL_CALLBACK_WRAPPER(onDoubleClick, (LLGLKey)keyCode);
        }

        void OnChar(Window& sender, wchar_t chr) override
        {
            LLGL_CALLBACK_WRAPPER(onChar, chr);
        }

        void OnWheelMotion(Window& sender, int motion) override
        {
            LLGL_CALLBACK_WRAPPER(onWheelMotion, motion);
        }

        void OnLocalMotion(Window& sender, const Offset2D& position) override
        {
            LLGL_CALLBACK_WRAPPER(onLocalMotion, (const LLGLOffset2D*)&position);
        }

        void OnGlobalMotion(Window& sender, const Offset2D& motion) override
        {
            LLGL_CALLBACK_WRAPPER(onGlobalMotion, (const LLGLOffset2D*)&motion);
        }

        void OnResize(Window& sender, const Extent2D& clientAreaSize) override
        {
            LLGL_CALLBACK_WRAPPER(onResize, (const LLGLExtent2D*)&clientAreaSize);
        }

        void OnUpdate(Window& sender) override
        {
            LLGL_CALLBACK_WRAPPER(onUpdate);
        }

        void OnGetFocus(Window& sender) override
        {
            LLGL_CALLBACK_WRAPPER(onGetFocus);
        }

        void OnLostFocus(Window& sender) override
        {
            LLGL_CALLBACK_WRAPPER(onLostFocus);
        }

};

using WindowEventListenerContainer = EventListenerContainer<InternalWindowEventListener, LLGLWindowEventListener>;

#undef LLGL_CALLBACK_WRAPPER

static std::vector<std::unique_ptr<Window>> g_Windows;
static WindowEventListenerContainer         g_WindowEventListenerContainer;

static void ConvertWindowDesc(WindowDescriptor& dst, const LLGLWindowDescriptor& src)
{
    dst.title               = src.title;
    dst.position            = { src.position.x, src.position.y };
    dst.size                = { src.size.width, src.size.height };
    dst.flags               = src.flags;
    dst.windowContext       = src.windowContext;
    dst.windowContextSize   = src.windowContextSize;
}

static void ConvertWindowDesc(LLGLWindowDescriptor& dst, const WindowDescriptor& src)
{
    static thread_local std::string internalTitle;
    internalTitle = src.title.c_str();
    dst.title               = internalTitle.c_str();
    dst.position            = { src.position.x, src.position.y };
    dst.size                = { src.size.width, src.size.height };
    dst.flags               = src.flags;
    dst.windowContext       = src.windowContext;
    dst.windowContextSize   = src.windowContextSize;
}

LLGL_C_EXPORT LLGLWindow llglCreateWindow(const LLGLWindowDescriptor* windowDesc)
{
    LLGL_ASSERT_PTR(windowDesc);
    WindowDescriptor internalWindowDesc;
    ConvertWindowDesc(internalWindowDesc, *windowDesc);
    g_Windows.push_back(Window::Create(internalWindowDesc));
    return LLGLWindow{ g_Windows.back().get() };
}

LLGL_C_EXPORT void llglReleaseWindow(LLGLWindow window)
{
    RemoveFromListIf(
        g_Windows,
        [window](const std::unique_ptr<Window>& entry) -> bool
        {
            return (entry.get() == LLGL_PTR(Window, window));
        }
    );
}

LLGL_C_EXPORT void llglSetWindowPosition(LLGLWindow window, const LLGLOffset2D* position)
{
    LLGL_PTR(Window, window)->SetPosition(*(const Offset2D*)position);
}

LLGL_C_EXPORT void llglGetWindowPosition(LLGLWindow window, LLGLOffset2D* outPosition)
{
    Offset2D position = LLGL_PTR(Window, window)->GetPosition();
    outPosition->x = position.x;
    outPosition->y = position.y;
}

LLGL_C_EXPORT void llglSetWindowSize(LLGLWindow window, const LLGLExtent2D* size, bool useClientArea)
{
    LLGL_PTR(Window, window)->SetSize(*(const Extent2D*)size, useClientArea);
}

LLGL_C_EXPORT void llglGetWindowSize(LLGLWindow window, LLGLExtent2D* outSize, bool useClientArea)
{
    Extent2D size = LLGL_PTR(Window, window)->GetSize(useClientArea);
    outSize->width = size.width;
    outSize->height = size.height;
}

LLGL_C_EXPORT void llglSetWindowTitle(LLGLWindow window, const wchar_t* title)
{
    LLGL_PTR(Window, window)->SetTitle(title);
}

LLGL_C_EXPORT void llglSetWindowTitleUTF8(LLGLWindow window, const char* title)
{
    LLGL_PTR(Window, window)->SetTitle(title);
}

LLGL_C_EXPORT size_t llglGetWindowTitle(LLGLWindow window, size_t outTitleLength, wchar_t* outTitle)
{
    UTF8String title = LLGL_PTR(Window, window)->GetTitle();
    SmallVector<wchar_t> titleUTF16 = title.to_utf16();
    if (outTitle != nullptr)
    {
        outTitleLength = std::min(outTitleLength, titleUTF16.size());
        ::memcpy(outTitle, titleUTF16.data(), outTitleLength * sizeof(wchar_t));
    }
    return titleUTF16.size();
}

LLGL_C_EXPORT size_t llglGetWindowTitleUTF8(LLGLWindow window, size_t outTitleLength, char* outTitle)
{
    UTF8String title = LLGL_PTR(Window, window)->GetTitle();
    if (outTitle != nullptr)
    {
        outTitleLength = std::min(outTitleLength, title.size() + 1);
        ::memcpy(outTitle, title.data(), outTitleLength * sizeof(char));
    }
    return title.size() + 1;
}

LLGL_C_EXPORT void llglShowWindow(LLGLWindow window, bool show)
{
    LLGL_PTR(Window, window)->Show(show);
}

LLGL_C_EXPORT bool llglIsWindowShown(LLGLWindow window)
{
    return LLGL_PTR(Window, window)->IsShown();
}

LLGL_C_EXPORT void llglSetWindowDesc(LLGLWindow window, const LLGLWindowDescriptor* windowDesc)
{
    LLGL_ASSERT_PTR(windowDesc);
    WindowDescriptor internalWindowDesc;
    ConvertWindowDesc(internalWindowDesc, *windowDesc);
    LLGL_PTR(Window, window)->SetDesc(internalWindowDesc);
}

LLGL_C_EXPORT void llglGetWindowDesc(LLGLWindow window, LLGLWindowDescriptor* outWindowDesc)
{
    LLGL_ASSERT_PTR(outWindowDesc);
    WindowDescriptor internalWindowDesc = LLGL_PTR(Window, window)->GetDesc();
    ConvertWindowDesc(*outWindowDesc, internalWindowDesc);
}

LLGL_C_EXPORT bool llglHasWindowFocus(LLGLWindow window)
{
    return LLGL_PTR(Window, window)->HasFocus();
}

LLGL_C_EXPORT bool llglHasWindowQuit(LLGLWindow window)
{
    return LLGL_PTR(Window, window)->HasQuit();
}

LLGL_C_EXPORT void llglSetWindowUserData(LLGLWindow window, void* userData)
{
    LLGL_PTR(Window, window)->SetUserData(userData);
}

LLGL_C_EXPORT void* llglGetWindowUserData(LLGLWindow window)
{
    return LLGL_PTR(const Window, window)->GetUserData();
}

LLGL_C_EXPORT int llglAddWindowEventListener(LLGLWindow window, const LLGLWindowEventListener* eventListener)
{
    auto eventListenerWrapper = g_WindowEventListenerContainer.Create(eventListener);
    LLGL_PTR(Window, window)->AddEventListener(eventListenerWrapper.second);
    return eventListenerWrapper.first;
}

LLGL_C_EXPORT void llglRemoveWindowEventListener(LLGLWindow window, int eventListenerID)
{
    if (auto eventListener = g_WindowEventListenerContainer.Release(eventListenerID))
        LLGL_PTR(Window, window)->RemoveEventListener(eventListener.get());
}

LLGL_C_EXPORT void llglPostWindowQuit(LLGLWindow window)
{
    LLGL_PTR(Window, window)->PostQuit();
}

LLGL_C_EXPORT void llglPostWindowKeyDown(LLGLWindow window, LLGLKey keyCode)
{
    LLGL_PTR(Window, window)->PostKeyDown((Key)keyCode);
}

LLGL_C_EXPORT void llglPostWindowKeyUp(LLGLWindow window, LLGLKey keyCode)
{
    LLGL_PTR(Window, window)->PostKeyUp((Key)keyCode);
}

LLGL_C_EXPORT void llglPostWindowDoubleClick(LLGLWindow window, LLGLKey keyCode)
{
    LLGL_PTR(Window, window)->PostDoubleClick((Key)keyCode);
}

LLGL_C_EXPORT void llglPostWindowChar(LLGLWindow window, wchar_t chr)
{
    LLGL_PTR(Window, window)->PostChar(chr);
}

LLGL_C_EXPORT void llglPostWindowWheelMotion(LLGLWindow window, int motion)
{
    LLGL_PTR(Window, window)->PostWheelMotion(motion);
}

LLGL_C_EXPORT void llglPostWindowLocalMotion(LLGLWindow window, const LLGLOffset2D* position)
{
    LLGL_PTR(Window, window)->PostLocalMotion(*(const Offset2D*)position);
}

LLGL_C_EXPORT void llglPostWindowGlobalMotion(LLGLWindow window, const LLGLOffset2D* motion)
{
    LLGL_PTR(Window, window)->PostGlobalMotion(*(const Offset2D*)motion);
}

LLGL_C_EXPORT void llglPostWindowResize(LLGLWindow window, const LLGLExtent2D* clientAreaSize)
{
    LLGL_PTR(Window, window)->PostResize(*(const Extent2D*)clientAreaSize);
}

LLGL_C_EXPORT void llglPostWindowUpdate(LLGLWindow window)
{
    LLGL_PTR(Window, window)->PostUpdate();
}

LLGL_C_EXPORT void llglPostWindowGetFocus(LLGLWindow window)
{
    LLGL_PTR(Window, window)->PostGetFocus();
}

LLGL_C_EXPORT void llglPostWindowLostFocus(LLGLWindow window)
{
    LLGL_PTR(Window, window)->PostLostFocus();
}


// } /namespace LLGL



// ================================================================================
