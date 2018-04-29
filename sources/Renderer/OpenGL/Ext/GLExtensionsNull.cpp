/*
 * GLExtensionsNull.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifdef LLGL_GL_ENABLE_EXT_PLACEHOLDERS


#include <stdexcept>
#include <string>


#ifndef __APPLE__

[[noreturn]]
static void ErrUnsupportedGLProc(const char* name)
{
    throw std::runtime_error("illegal use of unsupported OpenGL extension procedure: \"" + std::string(name) + "\"");
}

#endif


#define LLGL_DEF_GL_DUMMY_PROCS

#include "GLExtensionsNull.h"

#undef LLGL_DEF_GL_DUMMY_PROCS


#endif



// ================================================================================
