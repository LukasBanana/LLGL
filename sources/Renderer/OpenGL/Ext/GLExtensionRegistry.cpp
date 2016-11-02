/*
 * GLExtensionRegistry.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLExtensionRegistry.h"
#include <array>


namespace LLGL
{


static const unsigned int g_numExtensions = static_cast<int>(GLExt::Count);

static std::array<bool, g_numExtensions> g_extensionsEnabled { { false } };

void EnableExtensionSupport(GLExt extension)
{
    g_extensionsEnabled[static_cast<std::size_t>(extension)] = true;
}

bool HasExtension(const GLExt extension)
{
    return g_extensionsEnabled[static_cast<std::size_t>(extension)];
}


} // /namespace LLGL



// ================================================================================
