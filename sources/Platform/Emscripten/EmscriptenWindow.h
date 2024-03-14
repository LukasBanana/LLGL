/*
 * LinuxWindow.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EMSCRIPTEN_WINDOW_H
#define LLGL_EMSCRIPTEN_WINDOW_H


#include <LLGL/Window.h>
#include "EmscriptenDisplay.h"

namespace LLGL
{


class EmscriptenWindow : public Window
{

    public:

        EmscriptenWindow(const WindowDescriptor& desc);
        ~EmscriptenWindow();

        bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) override;

        void ResetPixelFormat() override;

        Extent2D GetContentSize() const override;

        void SetPosition(const Offset2D& position) override;
        Offset2D GetPosition() const override;

        void SetSize(const Extent2D& size, bool useClientArea = true) override;
        Extent2D GetSize(bool useClientArea = true) const override;

        void SetTitle(const UTF8String& title) override;
        UTF8String GetTitle() const override;

        void Show(bool show = true) override;
        bool IsShown() const override;

        void SetDesc(const WindowDescriptor& desc) override;
        WindowDescriptor GetDesc() const override;

    public:

        void ProcessEvent(/*XEvent& event*/);

    private:

        void CreateEmscriptenWindow();

        void ProcessKeyEvent(/*XKeyEvent& event, bool down*/);
        void ProcessMouseKeyEvent(/*XButtonEvent& event, bool down*/);
        void ProcessExposeEvent();
        void ProcessClientMessage(/*XClientMessageEvent& event*/);
        void ProcessMotionEvent(/*XMotionEvent& event*/);

        void PostMouseKeyEvent(Key key, bool down);
        
    private:
        static const char* OnBeforeUnloadCallback(int eventType, const void* reserved, void* userData);
        static int OnCanvasResizeCallback(int eventType, const EmscriptenUiEvent *keyEvent, void *userData);

        static EM_BOOL OnKeyCallback(int eventType, const EmscriptenKeyboardEvent *e, void *userData);

        static EM_BOOL OnMouseCallback(int eventType, const EmscriptenMouseEvent *e, void *userData);
        static EM_BOOL OnWheelCallback(int eventType, const EmscriptenWheelEvent *e, void *userData);

    private:
    
        WindowDescriptor            desc_;
        emscripten::val             canvas_;
        Offset2D                    prevMousePos_;

};


} // /namespace LLGL


#endif



// ================================================================================
