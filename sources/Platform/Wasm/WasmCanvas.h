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
#include <string>
#include <LLGL/Constants.h>


namespace LLGL
{


class WasmCanvas : public Canvas
{

    public:

        #include <LLGL/Backend/Canvas.inl>

    public:

        WasmCanvas(const CanvasDescriptor& desc);

    private:

        struct CanvasRect
        {
            double x;
            double y;
            double width;
            double height;
        };

    private:

        void CreateEmscriptenCanvas(const CanvasDescriptor& desc);
        void GetCanvasBoundingClientRect(CanvasRect& outRect) const;

        // Converts CSS client coordinates into canvas-relative physical pixel coordinates.
        Offset2D ClientToCanvasPosition(const CanvasRect& canvasRect, double clientX, double clientY) const;
        
        void PostResizeFromDOMCanvas();

    private:

        static const char* OnBeforeUnloadCallback(int eventType, const void* reserved, void* userData);
        static EM_BOOL OnOrientationChangeCallback(int eventType, const EmscriptenOrientationChangeEvent* event, void* userData);
        static EM_BOOL OnCanvasResizeCallback(int eventType, const EmscriptenUiEvent* event, void* userData);

        static EM_BOOL OnKeyCallback(int eventType, const EmscriptenKeyboardEvent* event, void* userData);

        static EM_BOOL OnMouseCallback(int eventType, const EmscriptenMouseEvent* event, void* userData);
        static EM_BOOL OnWheelCallback(int eventType, const EmscriptenWheelEvent* event, void* userData);

        static EM_BOOL OnTouchCallback(int eventType, const EmscriptenTouchEvent* event, void* userData);

    private:
    
        emscripten::val canvas_;
        std::string     canvasSelector_;
        int             canvasSize_[2]  = { 0, 0 };
        double          pixelRatio_     = 1.0;
        double          maxPixelRatio_  = 2.0;

        Offset2D        prevTouchPoint_[LLGL_MAX_NUM_INPUT_POINTERS];

};


} // /namespace LLGL


#endif



// ================================================================================
