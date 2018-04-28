/*
 * Strings.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_STRINGS_H
#define LLGL_STRINGS_H

/*
THIS HEADER MUST BE EXPLICITLY INCLUDED
*/

#include "Export.h"
#include "ShaderFlags.h"
#include "TextureFlags.h"


namespace LLGL
{


/**
\defgroup group_strings Global type-to-string conversion functions.
\addtogroup group_strings
@{
*/

//! Returns a string representation for the spcified ShaderType value, e.g. "vertex" or "tessellation control", or null if the input type is invalid.
LLGL_EXPORT const char* ToString(const ShaderType t);

/** @} */


} // /namespace LLGL


#endif



// ================================================================================
