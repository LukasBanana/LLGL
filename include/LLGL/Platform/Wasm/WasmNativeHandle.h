/*
 * WasmNativeHandle.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WASM_NATIVE_HANDLE_H
#define LLGL_WASM_NATIVE_HANDLE_H


#include <emscripten/val.h>


namespace LLGL
{


//! Emscripten native handle structure.
struct NativeHandle
{
    //! CSS selector of canvas object.
    emscripten::val canvas;
};


} // /namespace LLGL


#endif



// ================================================================================
