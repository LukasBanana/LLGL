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

/**
\brief Common move tag.
\remarks This can be used to explicitly copy a memory object like in the following example:
\code
// Allocate dynamic string object
char* s0 = new char[12];
std::strncpy(s0, "Hello World", 12);

// Don't initialize s1 as a weak reference to s0 and make it a copy instead.
LLGL::StringLiteral s1{ s0, LLGL::CopyTag{} };

// String s1 will still be valid beyond this point, because it made a copy of s0.
delete [] s0;
\endcode
*/
struct CopyTag{};


} // /namespace LLGL


#endif



// ================================================================================
