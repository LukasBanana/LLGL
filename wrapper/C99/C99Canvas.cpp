/*
 * C99Canvas.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Canvas.h>
#include <LLGL-C/Canvas.h>
#include "C99Internal.h"
#include "C99EventListenerContainer.h"
#include "../sources/Core/CoreUtils.h"
#include "../sources/Core/Exception.h"
#include <vector>
#include <string>
#include <string.h>


// namespace LLGL {


using namespace LLGL;

#define LLGL_CALLBACK_WRAPPER(FUNC, ...) \
    if (callbacks_.FUNC != NULL) { callbacks_.FUNC(LLGLCanvas{ &sender } LLGL_VA_ARGS(__VA_ARGS__)); }

class InternalCanvasEventListener final : public Canvas::EventListener
{

        LLGLCanvasEventListener callbacks_;

    public:

        InternalCanvasEventListener(const LLGLCanvasEventListener* callbacks)
        {
            memcpy(&callbacks_, callbacks, sizeof(LLGLCanvasEventListener));
        }

        void OnInit(Canvas& sender) override
        {
            LLGL_CALLBACK_WRAPPER(onInit);
        }

        void OnDestroy(Canvas& sender) override
        {
            LLGL_CALLBACK_WRAPPER(onDestroy);
        }

        void OnDraw(Canvas& sender) override
        {
            LLGL_CALLBACK_WRAPPER(onDraw);
        }

        void OnResize(Canvas& sender, const Extent2D& clientAreaSize) override
        {
            LLGL_CALLBACK_WRAPPER(onResize, (const LLGLExtent2D*)&clientAreaSize);
        }

        void OnTapGesture(Canvas& sender, const Offset2D& position, std::uint32_t numTouches) override
        {
            LLGL_CALLBACK_WRAPPER(onTapGesture, (const LLGLOffset2D*)&position, numTouches);
        }

        void OnPanGesture(Canvas& sender, const Offset2D& position, std::uint32_t numTouches, float dx, float dy, EventAction action) override
        {
            LLGL_CALLBACK_WRAPPER(onPanGesture, (const LLGLOffset2D*)&position, numTouches, dx, dy, (LLGLEventAction)action);
        }

        void OnKeyDown(Canvas& sender, Key keyCode) override
        {
            LLGL_CALLBACK_WRAPPER(onKeyDown, (LLGLKey)keyCode);
        }

        void OnKeyUp(Canvas& sender, Key keyCode) override
        {
            LLGL_CALLBACK_WRAPPER(onKeyUp, (LLGLKey)keyCode);
        }


};

using CanvasEventListenerContainer = EventListenerContainer<InternalCanvasEventListener, LLGLCanvasEventListener>;

#undef LLGL_CALLBACK_WRAPPER

static std::vector<std::unique_ptr<Canvas>> g_Canvases;
static CanvasEventListenerContainer         g_CanvasEventListenerContainer;

static void ConvertCanvasDesc(CanvasDescriptor& dst, const LLGLCanvasDescriptor& src)
{
    dst.title = src.title;
    dst.flags = src.flags;
}

LLGL_C_EXPORT LLGLCanvas llglCreateCanvas(const LLGLCanvasDescriptor* canvasDesc)
{
    LLGL_ASSERT_PTR(canvasDesc);
    CanvasDescriptor internalCanvasDesc;
    ConvertCanvasDesc(internalCanvasDesc, *canvasDesc);
    g_Canvases.push_back(Canvas::Create(internalCanvasDesc));
    return LLGLCanvas{ g_Canvases.back().get() };
}

LLGL_C_EXPORT void llglReleaseCanvas(LLGLCanvas canvas)
{
    RemoveFromListIf(
        g_Canvases,
        [canvas](const std::unique_ptr<Canvas>& entry) -> bool
        {
            return (entry.get() == LLGL_PTR(Canvas, canvas));
        }
    );
}

LLGL_C_EXPORT void llglSetCanvasTitle(LLGLCanvas canvas, const wchar_t* title)
{
    LLGL_PTR(Canvas, canvas)->SetTitle(title);
}

LLGL_C_EXPORT void llglSetCanvasTitleUTF8(LLGLCanvas canvas, const char* title)
{
    LLGL_PTR(Canvas, canvas)->SetTitle(title);
}

LLGL_C_EXPORT size_t llglGetCanvasTitle(LLGLCanvas canvas, size_t outTitleLength, wchar_t* outTitle)
{
    UTF8String title = LLGL_PTR(Canvas, canvas)->GetTitle();
    SmallVector<wchar_t> titleUTF16 = title.to_utf16();
    if (outTitle != NULL)
    {
        outTitleLength = (outTitleLength < titleUTF16.size() ? outTitleLength : titleUTF16.size());
        memcpy(outTitle, titleUTF16.data(), outTitleLength * sizeof(wchar_t));
    }
    return titleUTF16.size();
}

LLGL_C_EXPORT size_t llglGetCanvasTitleUTF8(LLGLCanvas canvas, size_t outTitleLength, char* outTitle)
{
    UTF8String title = LLGL_PTR(Canvas, canvas)->GetTitle();
    if (outTitle != nullptr)
    {
        outTitleLength = std::min(outTitleLength, title.size() + 1);
        ::memcpy(outTitle, title.data(), outTitleLength * sizeof(char));
    }
    return title.size() + 1;
}

LLGL_C_EXPORT bool llglHasCanvasQuit(LLGLCanvas canvas)
{
    return false; // deprecated
}

LLGL_C_EXPORT void llglSetCanvasUserData(LLGLCanvas canvas, void* userData)
{
    LLGL_PTR(Canvas, canvas)->SetUserData(userData);
}

LLGL_C_EXPORT void* llglGetCanvasUserData(LLGLCanvas canvas)
{
    return LLGL_PTR(const Canvas, canvas)->GetUserData();
}

LLGL_C_EXPORT int llglAddCanvasEventListener(LLGLCanvas canvas, const LLGLCanvasEventListener* eventListener)
{
    auto eventListenerWrapper = g_CanvasEventListenerContainer.Create(eventListener);
    LLGL_PTR(Canvas, canvas)->AddEventListener(eventListenerWrapper.second);
    return eventListenerWrapper.first;
}

LLGL_C_EXPORT void llglRemoveCanvasEventListener(LLGLCanvas canvas, int eventListenerID)
{
    if (auto eventListener = g_CanvasEventListenerContainer.Release(eventListenerID))
        LLGL_PTR(Canvas, canvas)->RemoveEventListener(eventListener.get());
}

LLGL_C_EXPORT void llglPostCanvasQuit(LLGLCanvas canvas)
{
    // deprecated
}

LLGL_C_EXPORT void llglPostCanvasInit(LLGLCanvas sender)
{
    LLGL_PTR(Canvas, sender)->PostInit();
}

LLGL_C_EXPORT void llglPostCanvasDestroy(LLGLCanvas sender)
{
    LLGL_PTR(Canvas, sender)->PostDestroy();
}

LLGL_C_EXPORT void llglPostCanvasDraw(LLGLCanvas sender)
{
    LLGL_PTR(Canvas, sender)->PostDraw();
}

LLGL_C_EXPORT void llglPostCanvasResize(LLGLCanvas sender, const LLGLExtent2D* clientAreaSize)
{
    LLGL_PTR(Canvas, sender)->PostResize(*(const Extent2D*)clientAreaSize);
}

LLGL_C_EXPORT void llglPostCanvasTapGesture(LLGLCanvas sender, const LLGLOffset2D* position, uint32_t numTouches)
{
    LLGL_PTR(Canvas, sender)->PostTapGesture(*(const Offset2D*)position, numTouches);
}

LLGL_C_EXPORT void llglPostCanvasPanGesture(LLGLCanvas sender, const LLGLOffset2D* position, uint32_t numTouches, float dx, float dy, LLGLEventAction action)
{
    LLGL_PTR(Canvas, sender)->PostPanGesture(*(const Offset2D*)position, numTouches, dx, dy, (EventAction)action);
}

LLGL_C_EXPORT void llglPostCanvasKeyDown(LLGLCanvas sender, LLGLKey keyCode)
{
    LLGL_PTR(Canvas, sender)->PostKeyDown((Key)keyCode);
}

LLGL_C_EXPORT void llglPostCanvasKeyUp(LLGLCanvas sender, LLGLKey keyCode)
{
    LLGL_PTR(Canvas, sender)->PostKeyUp((Key)keyCode);
}



// } /namespace LLGL



// ================================================================================
