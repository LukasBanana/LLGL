/*
 * GLCore.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_CORE_H
#define LLGL_GL_CORE_H


#include "GLImport.h"
#include <string>


namespace LLGL
{


// Throws an std::runtime_error exception if 'status' is not equal to 'statusRequired'.
void GLThrowIfFailed(const GLenum status, const GLenum statusRequired, const char* info);

// Converts the GL debug source into a string.
std::string GLDebugSourceToStr(const GLenum source);

// Converts the GL debug type into a string.
std::string GLDebugTypeToStr(const GLenum type);

// Converts the GL debug severity into a string.
std::string GLDebugSeverityToStr(const GLenum severity);

// Converts the boolean value into a GLboolean value.
GLboolean GLBoolean(bool value);


} // /namespace LLGL


#endif



// ================================================================================
