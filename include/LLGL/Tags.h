/*
 * Tags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_TAGS_H
#define LLGL_TAGS_H


namespace LLGL
{


/**
\brief Common uninitialize tag.
\remarks This can be used to explicitly construct an uninitialized color or even an entire container of uninitialized colors:
\code
// Explicitly uninitialized color.
LLGL::ColorRGBAf color { Gs::UninitializeTag{} };

// Explicitly uninitialized color elements in a container.
std::vector<LLGL::ColorRGBAf> color;
color.resize(1024, LLGL::ColorRGBAf { Gs::UninitializeTag{} });
\endcode
*/
struct UninitializeTag {};


} // /namespace LLGL


#endif



// ================================================================================
