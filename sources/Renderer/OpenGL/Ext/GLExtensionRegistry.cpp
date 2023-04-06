/*
 * GLExtensionRegistry.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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

bool HasNativeSamplers()
{
    return HasExtension(GLExt::ARB_sampler_objects);
}

bool HasNativeVAO()
{
    return HasExtension(GLExt::ARB_vertex_array_object);
}


} // /namespace LLGL



// ================================================================================
