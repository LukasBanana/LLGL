/*
 * WasmCanvas.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WASM_CANVAS_H
#define LLGL_WASM_CANVAS_H


#include <LLGL/Canvas.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>
#include <emscripten/key_codes.h>


namespace LLGL
{


class WasmCanvas : public Canvas
{

    public:

        #include <LLGL/Backend/Canvas.inl>

    public:

        WasmCanvas(const CanvasDescriptor& desc);

    private:

        void CreateEmscriptenCanvas(const CanvasDescriptor& desc);
        
    private:

        static const char* OnBeforeUnloadCallback(int eventType, const void* reserved, void* userData);
        static EM_BOOL OnCanvasResizeCallback(int eventType, const EmscriptenUiEvent* event, void* userData);

        static EM_BOOL OnKeyCallback(int eventType, const EmscriptenKeyboardEvent* event, void* userData);

        static EM_BOOL OnMouseCallback(int eventType, const EmscriptenMouseEvent* event, void* userData);
        static EM_BOOL OnWheelCallback(int eventType, const EmscriptenWheelEvent* event, void* userData);

        static EM_BOOL OnTouchCallback(int eventType, const EmscriptenTouchEvent* event, void* userData);

    private:
    
        emscripten::val canvas_;
        long            prevTouchPoint_[2] = { 0, 0 };

};


} // /namespace LLGL


#endif



// ================================================================================
