/*
 * GLExtensionRegistry.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLExtensionRegistry.h"
#include <array>


namespace LLGL
{


#ifdef __APPLE__

void RegisterExtension(GLExt extension)
{
    // dummy
}

bool HasExtension(const GLExt extension)
{
    /*
    MacOS does not support dynamic OpenGL extension loaded; all supported extensions are included per default.
    Thus, always return true on MacOS; unsupported extensions are excluded by pre-processor directives.
    */
    return true;
}

#else

static std::array<bool, static_cast<std::size_t>(GLExt::Count)> g_registeredExtensions { { false } };

void RegisterExtension(GLExt extension)
{
    g_registeredExtensions[static_cast<std::size_t>(extension)] = true;
}

bool HasExtension(const GLExt extension)
{
    return g_registeredExtensions[static_cast<std::size_t>(extension)];
}

#endif


} // /namespace LLGL



// ================================================================================
