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


/**
\brief Motion event action enumeration.
\see Canvas::EventListener::OnPanGesture
\see Canvas::PostPanGesture
*/
enum class EventAction
{
    Began,      //!< Action when a gesture began.
    Changed,    //!< Action when a gesture changed/moved.
    Ended,      //!< Action when a gesture ended.
};

/**
\brief Canvas creation flags.
\see CanvasDescriptor::flags
*/
struct CanvasFlags
{
    enum
    {
        //! Specifies whether the canvas is borderless. This is required for a fullscreen swap-chain.
        Borderless = (1 << 0),
    };
};

/**
\brief Canvas descriptor structure.
\see Canvas::Create
*/
struct CanvasDescriptor
{
    //! Canvas title as UTF16 STL::string.
    UTF8String  title;

    /**
    \brief Specifies the canvas creation flags. This can be a bitwise OR combination of the CanvasFlags entries.
    \see CanvasFlags
    */
    long        flags = 0;
};


} // /namespace LLGL


#endif



// ================================================================================
