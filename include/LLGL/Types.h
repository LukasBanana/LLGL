/*
 * Types.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_TYPES_H
#define LLGL_TYPES_H


#include <Gauss/Vector2.h>


namespace LLGL
{


/**
\brief 2D point (integer).
\todo Rename to "Offset2D" and make it a simple struct.
*/
using Point = Gs::Vector2i;

/**
\brief 2D size (integer).
\todo Rename to "Extent2D" and make it a simple struct.
*/
using Size = Gs::Vector2i;


} // /namespace LLGL


#endif



// ================================================================================
