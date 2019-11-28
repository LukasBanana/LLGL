/*
 * GLExtensionRegistry.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLExtensionRegistry.h"
#include <array>


namespace LLGL
{


static std::array<bool, static_cast<std::size_t>(GLExt::Count)> g_registeredExtensions { { false } };

void RegisterExtension(GLExt extension)
{
    g_registeredExtensions[static_cast<std::size_t>(extension)] = true;
}

bool HasExtension(const GLExt extension)
{
    return g_registeredExtensions[static_cast<std::size_t>(extension)];
}


} // /namespace LLGL



// ================================================================================
