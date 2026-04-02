/*
 * D3D9ModuleInterface.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../ModuleInterface.h"
#include "D3D9RenderSystem.h"


LLGL_IMPLEMENT_RENDERER_MODULE(Direct3D9, "Direct3D 9", LLGL::RendererID::Direct3D9, LLGL::D3D9RenderSystem, 1009);



// ================================================================================
