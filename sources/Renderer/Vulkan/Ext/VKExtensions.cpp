/*
 * VKExtensions.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKExtensions.h"


namespace LLGL
{


/* ~~~~~ Define all VK extension functions ~~~~~ */

#define DECL_VKPROC(NAME) \
    PFN_##NAME NAME = nullptr

// Include inline header for object definitions
#include "VKExtensionsDecl.inl"

#undef DECL_VKPROC


} // /namespace LLGL



// ================================================================================
