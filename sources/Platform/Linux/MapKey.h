/*
 * MapKey.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MAP_KEY_H
#define LLGL_MAP_KEY_H


#include <LLGL/Key.h>
#include <X11/Xlib.h>


namespace LLGL
{


Key MapKey(XKeyEvent& keyEvent);


} // /namespace LLGL


#endif



// ================================================================================
