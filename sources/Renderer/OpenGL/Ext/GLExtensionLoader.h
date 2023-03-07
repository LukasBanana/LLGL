/*
 * GLExtensionLoader.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_EXTENSION_LOADER_H
#define LLGL_GL_EXTENSION_LOADER_H


namespace LLGL
{


/*
Loads all suported OpenGL extensions (suported by both the OpenGL server and LLGL) and returns true on success.
Otherwise, at least one extension was erroneously reported as available while their respective procedures could not be loaded.
*/
bool LoadSupportedOpenGLExtensions(bool isCoreProfile);

// Returns true if all available extensions have been loaded.
bool AreOpenGLExtensionsLoaded();

/* --- Common GL extensions --- */

#ifndef __APPLE__

bool LoadSwapIntervalProcs();
bool LoadPixelFormatProcs();
bool LoadCreateContextProcs();

#endif


} // /namespace LLGL


#endif



// ================================================================================
