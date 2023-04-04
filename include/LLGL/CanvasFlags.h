/*
 * CanvasFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_CANVAS_FLAGS_H
#define LLGL_CANVAS_FLAGS_H


#include <LLGL/Container/Strings.h>


namespace LLGL
{


//! Canvas descriptor structure.
struct CanvasDescriptor
{
    //! Canvas title as UTF16 string.
    UTF8String  title;

    //! Specifies whether the canvas is borderless. This is required for a fullscreen swap-chain.
    bool        borderless = false;
};


} // /namespace LLGL


#endif



// ================================================================================
