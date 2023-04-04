/*
 * Version.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VERSION_H
#define LLGL_VERSION_H


#include <LLGL/Export.h>


namespace LLGL
{

//! Namespace with functions to determine LLGL version.
namespace Version
{


//! Returns the major LLGL version (e.g. 1 stands for "1.00").
LLGL_EXPORT unsigned GetMajor();

//! Returns the minor LLGL version (e.g. 1 stands for "0.01"). Must be less than 100.
LLGL_EXPORT unsigned GetMinor();

//! Returns the revision version number. Must be less than 100.
LLGL_EXPORT unsigned GetRevision();

//! Returns the LLGL version status (either "Alpha", "Beta", or empty).
LLGL_EXPORT const char* GetStatus();

//! Returns the full LLGL version as an ID number (e.g. 200317 stands for "2.03 (Rev. 17)").
LLGL_EXPORT unsigned GetID();

//! Returns the full LLGL version as a string (e.g. "0.01 Beta (Rev. 1)").
LLGL_EXPORT const char* GetString();


} // /namespace Version

} // /namespace LLGL


#endif



// ================================================================================
