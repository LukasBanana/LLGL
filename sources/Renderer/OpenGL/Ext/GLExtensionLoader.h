/*
 * GLExtensionLoader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_EXTENSION_LOADER_H
#define LLGL_GL_EXTENSION_LOADER_H


#include <set>


namespace LLGL
{


/*
Loads all suported OpenGL extensions (suported by both the OpenGL server and LLGL) and returns true on success.
Otherwise, at least one extension was erroneously reported as available while their respective procedures could not be loaded.
*/
bool LoadSupportedOpenGLExtensions(bool isCoreProfile, bool abortOnFailure = false);

// Returns true if all available extensions have been loaded.
bool AreOpenGLExtensionsLoaded();

// Returns the set of OpenGL extensions that are supported by the GL context that was active during the last call to LoadSupportedOpenGLExtensions().
const std::set<const char*>& GetSupportedOpenGLExtensions();

// Returns the set of OpenGL extensions that were loaded during the last call to LoadSupportedOpenGLExtensions().
const std::set<const char*>& GetLoadedOpenGLExtensions();

/* --- Common GL extensions --- */

#ifndef __APPLE__

bool LoadSwapIntervalProcs();
bool LoadPixelFormatProcs();
bool LoadCreateContextProcs();

#endif


} // /namespace LLGL


#endif



// ================================================================================
