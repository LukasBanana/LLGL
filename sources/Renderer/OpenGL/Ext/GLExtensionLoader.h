/*
 * GLExtensionLoader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_EXTENSION_LOADER_H
#define LLGL_GL_EXTENSION_LOADER_H


#include "../../GLCommon/GLExtensionRegistry.h"
#include <string>
#include <array>
#include <set>


namespace LLGL
{


//! OpenGL extension map type.
using GLExtensionList = std::set<std::string>;

/* --- Common extension loading functions --- */

/**
Returns a hash-map with all supported OpenGL extensions.
The hash-map can be used for faster single-extension queries.
\param[in] coreProfile Specifies whether the extension are to be loaded via GL core profile or not.
*/
GLExtensionList QueryExtensions(bool coreProfile);

/**
Loads all available extensions and prints errors if an extension is available,
but their respective functions could not be loaded.
\param[in,out] extMap Specifies the extension map. This can be queried by the "QueryExtensions" function.
If an extension is available but some of their respective functions could not be loaded,
the respective entry in the map will be invalidated (set to 'false').
\see QueryExtensions
*/
void LoadAllExtensions(GLExtensionList& extensions);

//! Returns true if all available extensions have been loaded.
bool AreExtensionsLoaded();

/* --- Common GL extensions --- */

#ifndef __APPLE__

bool LoadSwapIntervalProcs();
bool LoadPixelFormatProcs();
bool LoadCreateContextProcs();

#endif


} // /namespace LLGL


#endif



// ================================================================================
