/*
 * VKExtensions.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_EXTENSIONS_H
#define LLGL_VK_EXTENSIONS_H


#include "../Vulkan.h"


namespace LLGL
{



/* ~~~~~ Define all VK extension functions ~~~~~ */

#define DECL_VKPROC(NAME) \
    extern PFN_##NAME NAME

// Include inline header for object declarations
#include "VKExtensionsDecl.inl"

#undef DECL_VKPROC


} // /namespace LLGL


#endif



// ================================================================================
