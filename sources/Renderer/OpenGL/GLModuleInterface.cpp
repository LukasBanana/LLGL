/*
 * GLModuleInterface.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../ModuleInterface.h"
#include "GLRenderSystem.h"
#include "Profile/GLProfile.h"


/*
Use indirection to solve macro expansion dependency.
LLGL_OPENGL_PROFILE nees to be expanded before begin passed to LLGL_IMPLEMENT_RENDERER_MODULE().
*/
#define LLGL_IMPLEMENT_RENDERER_MODULE_OPENGL(PROFILE) \
    LLGL_IMPLEMENT_RENDERER_MODULE(PROFILE, LLGL::GLProfile::GetRendererName(), LLGL::GLProfile::GetRendererID(), LLGL::GLRenderSystem, 100)

LLGL_IMPLEMENT_RENDERER_MODULE_OPENGL(LLGL_OPENGL_PROFILE);



// ================================================================================
