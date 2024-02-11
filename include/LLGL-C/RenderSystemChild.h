/*
 * RenderSystemChild.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_RENDER_SYSTEM_CHILD_H
#define LLGL_C99_RENDER_SYSTEM_CHILD_H


#include <LLGL-C/Export.h>
#include <LLGL-C/LLGLWrapper.h>


LLGL_C_EXPORT void llglSetDebugName(LLGLRenderSystemChild renderSystemChild, const char* name);

//! \deprecated Since 0.04b; Use llglSetDebugName instead.
LLGL_C_EXPORT void llglSetName(LLGLRenderSystemChild renderSystemChild, const char* name);


#endif



// ================================================================================
