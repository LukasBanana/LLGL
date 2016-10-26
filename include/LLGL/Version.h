/*
 * Version.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VERSION_H
#define LLGL_VERSION_H


#include "Export.h"
#include <string>


namespace LLGL
{

namespace Version
{


//! Returns the major LLGL version (e.g. 1 stands for "1.00").
LLGL_EXPORT unsigned int GetMajor();

//! Returns the minor LLGL version (e.g. 1 stands for "0.01"). Must be less than 100.
LLGL_EXPORT unsigned int GetMinor();

//! Returns the revision version number. Must be less than 100.
LLGL_EXPORT unsigned int GetRevision();

//! Returns the LLGL version status (either "Alpha", "Beta", or empty).
LLGL_EXPORT std::string GetStatus();

//! Returns the full LLGL version as an ID number (e.g. 200317 stands for "2.03 (Rev. 17)").
LLGL_EXPORT unsigned int GetID();

//! Returns the full LLGL version as a string (e.g. "0.01 Beta (Rev. 1)").
LLGL_EXPORT std::string GetString();


} // /namespace Version

} // /namespace LLGL


#endif



// ================================================================================
