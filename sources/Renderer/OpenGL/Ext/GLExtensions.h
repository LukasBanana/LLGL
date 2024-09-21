/*
 * GLExtensions.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_EXTENSIONS_H
#define LLGL_GL_EXTENSIONS_H


#if LLGL_OPENGL
#   if LLGL_GL_ENABLE_OPENGL2X
#       include "../Profile/GLCompat/GLCompatExtensions.h"
#   else
#       include "../Profile/GLCore/GLCoreExtensions.h"
#   endif
#elif LLGL_OPENGLES3
#   include "../Profile/GLES/GLESExtensions.h"
#endif


#endif



// ================================================================================
