/*
 * EmscriptenCanvas.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EMSCRIPTEN_CANVAS_H
#define LLGL_EMSCRIPTEN_CANVAS_H


#include <LLGL/Canvas.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>
#include <emscripten/key_codes.h>


namespace LLGL
{


class EmscriptenCanvas : public Canvas
{

    public:

        EmscriptenCanvas(const CanvasDescriptor& desc);

        bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) override;

        Extent2D GetContentSize() const override;

        void SetTitle(const UTF8String& title) override;
        UTF8String GetTitle() const override;

    private:

        void CreateEmscriptenCanvas(const CanvasDescriptor& desc);

        void ProcessKeyEvent(/*event, bool down*/);
        void ProcessMouseKeyEvent(/*event, bool down*/);
        //void ProcessExposeEvent();
        void ProcessClientMessage(/*event*/);
        void ProcessMotionEvent(/*event*/);

        void PostMouseKeyEvent(Key key, bool down);
        
    private:

        static const char* OnBeforeUnloadCallback(int eventType, const void* reserved, void* userData);
        static int OnCanvasResizeCallback(int eventType, const EmscriptenUiEvent *keyEvent, void *userData);

        static EM_BOOL OnKeyCallback(int eventType, const EmscriptenKeyboardEvent *e, void *userData);

        static EM_BOOL OnMouseCallback(int eventType, const EmscriptenMouseEvent *e, void *userData);
        static EM_BOOL OnWheelCallback(int eventType, const EmscriptenWheelEvent *e, void *userData);

    private:
    
        emscripten::val canvas_;

};


} // /namespace LLGL


#endif



// ================================================================================
