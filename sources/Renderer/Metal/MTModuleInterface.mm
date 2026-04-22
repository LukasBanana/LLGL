/*
 * MTModuleInterface.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../ModuleInterface.h"
#include "MTRenderSystem.h"


LLGL_IMPLEMENT_RENDERER_MODULE(Metal, "Metal", LLGL::RendererID::Metal, LLGL::MTRenderSystem, 300);



// ================================================================================
