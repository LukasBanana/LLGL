/*
 * GLExtensionLoader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_EXTENSION_LOADER_H
#define LLGL_GL_EXTENSION_LOADER_H


#include "GLExtensionRegistry.h"
#include <string>
#include <array>
#include <map>


namespace LLGL
{


//! OpenGL extension map type.
using GLExtensionList = std::map<std::string, bool>;

/* --- Common extension loading functions --- */

/*
Returns a hash-map with all supported OpenGL extensions.
The hash-map can be used for faster single-extension queries.
Parameter <coreProfile> specifies whether the extension are to be loaded via GL core profile or not.
*/
GLExtensionList QueryExtensions(bool coreProfile);

/*
Loads all available extensions and prints errors if an extension is available,
but their respective functions could not be loaded.
Parameter <extensions> specifies the extension map. This can be queried by the "QueryExtensions" function.
The respective entry will be set to true if all its functions have been loaded successfully.
*/
void LoadAllExtensions(GLExtensionList& extensions, bool coreProfile);

// Returns true if all available extensions have been loaded.
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
