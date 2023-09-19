/*
 * Path.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_PATH_H
#define LLGL_PATH_H


#include <LLGL/Export.h>
#include <LLGL/Container/UTF8String.h>
#include <string>


namespace LLGL
{

// Namespace with abstract platform functions for resource paths and file system
namespace Path
{


// Returns the platform specific path separator, i.e. either '\\' on Windows or '/' on all other platforms.
LLGL_EXPORT char GetSeparator();

// Sanitizes the specified path:
//  - Replaces wrong separators with the appropriate one for the host platform.
//  - Replaces redundant upper-level directory entries, e.g. "Foo/../Bar/" to "Bar".
//  - Strips trailing separators.
LLGL_EXPORT UTF8String Sanitize(const UTF8String& path);

// Combines the two specified paths. Trailing '\\' and '/' characters will be stripped.
LLGL_EXPORT UTF8String Combine(const UTF8String& lhs, const UTF8String& rhs);

// Returns the current working directory for the active process.
LLGL_EXPORT UTF8String GetWorkingDir();

// Returns the input filename as absolute path.
LLGL_EXPORT UTF8String GetAbsolutePath(const UTF8String& filename);


} // /nameapace Path

} // /namespace LLGL


#endif



// ================================================================================
