/*
 * GLESExtensionsProxy.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifdef LLGL_GL_ENABLE_EXT_PLACEHOLDERS


#include "../OpenGL.h"
#include "../GLCore.h"
#include <stdexcept>
#include <string>



namespace LLGL
{


#define LLGL_DEF_GLES_PROXY_PROCS

// Include inline header for proxy function definitions
#include "GLESExtensionsDecl.inl"

#undef LLGL_DEF_GLES_PROXY_PROCS


} // /namespace LLGL


#endif // /LLGL_GL_ENABLE_EXT_PLACEHOLDERS



// ================================================================================
