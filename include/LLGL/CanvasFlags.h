/*
 * CanvasFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_CANVAS_FLAGS_H
#define LLGL_CANVAS_FLAGS_H


#include <string>


namespace LLGL
{


//! Canvas descriptor structure.
struct CanvasDescriptor
{
    //! Canvas title as UTF16 string.
    std::wstring    title;

    //! Specifies whether the canvas is borderless. This is required for a fullscreen render context.
    bool            borderless = false;
};


} // /namespace LLGL


#endif



// ================================================================================
