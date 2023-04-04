/*
 * Tags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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
LLGL::ColorRGBAf color{ LLGL::UninitializeTag{} };

// Explicitly uninitialized color elements in a container.
std::vector<LLGL::ColorRGBAf> color;
color.resize(1024, LLGL::ColorRGBAf{ LLGL::UninitializeTag{} });
\endcode
*/
struct UninitializeTag{};


} // /namespace LLGL


#endif



// ================================================================================
