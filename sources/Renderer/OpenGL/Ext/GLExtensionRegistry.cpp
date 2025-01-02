/*
 * GLExtensionRegistry.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLExtensionRegistry.h"


namespace LLGL
{


static bool g_registeredExtensions[static_cast<std::size_t>(GLExt::Count)] = {};

static void RegisterExtensionInternal(GLExt extension, bool enabled = true)
{
    g_registeredExtensions[static_cast<std::size_t>(extension)] = enabled;
}

void RegisterExtension(GLExt extension)
{
    #if LLGL_GL_ENABLE_OPENGL2X
    switch (extension)
    {
        case GLExt::EXT_framebuffer_object:
            RegisterExtensionInternal(GLExt::ARB_framebuffer_object); // Substitute EXT_framebuffer_object with ARB_framebuffer_object
            break;
        default:
            RegisterExtensionInternal(extension);
            break;
    }
    #else // LLGL_GL_ENABLE_OPENGL2X
    RegisterExtensionInternal(extension);
    #endif // /LLGL_GL_ENABLE_OPENGL2X
}

void DisableIncompatibleExtensions()
{
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* The following extensions must be supported with DSA. Otherwise, something is misconfigured and it has to be disabled */
        for (GLExt ext : {
                GLExt::ARB_texture_storage,
                GLExt::ARB_texture_storage_multisample,
            })
        {
            if (!HasExtension(ext))
            {
                RegisterExtensionInternal(GLExt::ARB_direct_state_access, false);
                break;
            }
        }
    }
}

bool HasExtension(const GLExt extension)
{
    return g_registeredExtensions[static_cast<std::size_t>(extension)];
}

bool HasNativeSamplers()
{
    return HasExtension(GLExt::ARB_sampler_objects);
}


} // /namespace LLGL



// ================================================================================
