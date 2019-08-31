/*
 * Strings.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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
#include "RenderingDebugger.h"
#include "RenderSystemFlags.h"


namespace LLGL
{


/**
\defgroup group_strings Global type-to-string conversion functions.
\addtogroup group_strings
@{
*/

/**
\brief Returns a string representation for the spcified ShaderType value, or null if the input type is invalid.
\remarks Return value examples are \c "vertex", \c "tessellation control".
*/
LLGL_EXPORT const char* ToString(const ShaderType t);

/**
\brief Returns a string representation for the specified ErrorType value, or null if the input type is invalid.
\remarks Return value examples are \c "invalid argument", \c "unsupported feature".
*/
LLGL_EXPORT const char* ToString(const ErrorType t);

/**
\brief Returns a string representation for the specified WarningType value, or null if the input type is invalid.
\remarks Return value examples are \c "improper argument", \c "pointless operation".
*/
LLGL_EXPORT const char* ToString(const WarningType t);

/**
\brief Returns a string representation for the specified ShadingLanguage value, or null if the input type is invalid.
\remarks Return value examples are \c "GLSL 450", \c "HLSL 2.0c".
*/
LLGL_EXPORT const char* ToString(const ShadingLanguage t);

/**
\brief Returns a string representation for the specified Format value, or null if the input type is invalid.
\remarks Return value examples are \c "R8UNorm", \c "RGBA16Float", \c "D24UNormS8UInt", \c "RGB DXT1".
*/
LLGL_EXPORT const char* ToString(const Format t);

/**
\brief Returns a string representation for the specified TextureType value, or null if the input type is invalid.
\remarks Return value examples are \c "Texture1D", \c "Texture2DArray".
*/
LLGL_EXPORT const char* ToString(const TextureType t);

/** @} */


} // /namespace LLGL


#endif



// ================================================================================
