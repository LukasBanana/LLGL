/*
 * C99Canvas.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Canvas.h>
#include <LLGL-C/Canvas.h>
#include "C99Internal.h"
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

        void OnProcessEvents(Canvas& sender) override
        {
            LLGL_CALLBACK_WRAPPER(onProcessEvents);
        }

        void OnQuit(Canvas& sender, bool& veto) override
        {
            LLGL_CALLBACK_WRAPPER(onQuit, &veto);
        }

};

#undef LLGL_CALLBACK_WRAPPER

static std::vector<std::unique_ptr<Canvas>> g_Canvases;

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

LLGL_C_EXPORT bool llglHasCanvasQuit(LLGLCanvas canvas)
{
    return LLGL_PTR(Canvas, canvas)->HasQuit();
}

LLGL_C_EXPORT int llglAddCanvasEventListener(LLGLCanvas canvas, const LLGLCanvasEventListener* eventListener)
{
    return 0; //todo
}

LLGL_C_EXPORT void llglRemoveCanvasEventListener(LLGLCanvas canvas, int eventListenerID)
{
    //todo
}

LLGL_C_EXPORT void llglPostCanvasQuit(LLGLCanvas canvas)
{
    LLGL_PTR(Canvas, canvas)->PostQuit();
}


// } /namespace LLGL



// ================================================================================
