/*
 * GLCore.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_CORE_H__
#define __LLGL_GL_CORE_H__


#include "OpenGL.h"
#include <string>


namespace LLGL
{


//! Converts the GL error code into a string.
std::string GLErrorToStr(const GLenum errorCode);

//! Converts the GL debug source into a string.
std::string GLDebugSourceToStr(const GLenum source);

//! Converts the GL debug type into a string.
std::string GLDebugTypeToStr(const GLenum type);

//! Converts the GL debug severity into a string.
std::string GLDebugSeverityToStr(const GLenum severity);

//! Converts the boolean value into a GLboolean value.
GLboolean GLBoolean(bool value);


} // /namespace LLGL


#endif



// ================================================================================
