/*
 * TypeNames.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_TYPE_NAMES_H
#define LLGL_TYPE_NAMES_H

/*
THIS HEADER MUST BE EXPLICITLY INCLUDED
*/

#include <LLGL/Export.h>
#include <LLGL/Format.h>
#include <LLGL/ImageFlags.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/ResourceFlags.h>
#include <LLGL/RenderingDebugger.h>
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/QueryHeapFlags.h>


namespace LLGL
{


/**
\defgroup group_strings Global type-to-string conversion functions.
\addtogroup group_strings
@{
*/

/**
\brief Returns a string representation for the specified ShaderType value, or null if the input type is invalid.
\remarks Return value examples are \c "vertex", \c "tessellation control".
*/
LLGL_EXPORT const char* ToString(const ShaderType val);

/**
\brief Returns a string representation for the specified ErrorType value, or null if the input type is invalid.
\remarks Return value examples are \c "invalid argument", \c "unsupported feature".
*/
LLGL_EXPORT const char* ToString(const ErrorType val);

/**
\brief Returns a string representation for the specified WarningType value, or null if the input type is invalid.
\remarks Return value examples are \c "improper argument", \c "pointless operation".
*/
LLGL_EXPORT const char* ToString(const WarningType val);

/**
\brief Returns a string representation for the specified ShadingLanguage value, or null if the input type is invalid.
\remarks Return value examples are \c "GLSL 450", \c "HLSL 2.0c".
*/
LLGL_EXPORT const char* ToString(const ShadingLanguage val);

/**
\brief Returns a string representation for the specified Format value, or null if the input type is invalid.
\remarks Return value examples are \c "R8UNorm", \c "RGBA16Float", \c "D24UNormS8UInt", \c "RGB DXT1".
*/
LLGL_EXPORT const char* ToString(const Format val);

/**
\brief Returns a string representation for the specified ImageFormat value, or null if the input type is invalid.
\remarks Return value examples are \c "RGB", \c "ABGR".
*/
LLGL_EXPORT const char* ToString(const ImageFormat val);

/**
\brief Returns a string representation for the specified TextureType value, or null if the input type is invalid.
\remarks Return value examples are \c "Texture1D", \c "Texture2DArray".
*/
LLGL_EXPORT const char* ToString(const TextureType val);

/**
\brief Returns a string representation for the specified BlendOp value, or null if the input type is invalid.
\remarks Return value examples are \c "Zero", \c "SrcAlpha".
*/
LLGL_EXPORT const char* ToString(const BlendOp val);

/**
\brief Returns a string representation for the specified ResourceType value, or null if the input type is invalid.
\remarks Return value examples are \c "buffer", \c "texture".
*/
LLGL_EXPORT const char* ToString(const ResourceType val);

/**
\brief Returns a string representation for the specified SystemValue value, or null if the input type is invalid.
\remarks Return value examples are \c "Position", \c "ClipDistance".
*/
LLGL_EXPORT const char* ToString(const SystemValue val);

/**
\brief Returns a string representation for the specified QueryType value, or null if the input type is invalid.
\remarks Return value examples are \c "SamplesPassed", \c "AnySamplesPassed".
*/
LLGL_EXPORT const char* ToString(const QueryType val);

/** @} */


} // /namespace LLGL


#endif



// ================================================================================
