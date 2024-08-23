/*
 * WasmKeyCodes.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WASM_KEY_CODES_H
#define LLGL_WASM_KEY_CODES_H


#include <LLGL/Key.h>


namespace LLGL
{


Key MapEmscriptenKeyCode(const char* keyEvent);


} // /namespace LLGL


#endif



// ================================================================================
