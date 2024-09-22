/*
 * GLCore.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_CORE_H
#define LLGL_GL_CORE_H


#include "OpenGL.h"
#include <string>


namespace LLGL
{


// Traps program execution if 'status' is not equal to 'statusRequired'.
void GLThrowIfFailed(const GLenum status, const GLenum statusRequired, const char* info = nullptr);

// Converts the GL debug source into a string.
const char* GLDebugSourceToStr(const GLenum source);

// Converts the GL debug type into a string.
const char* GLDebugTypeToStr(const GLenum type);

// Converts the GL debug severity into a string.
const char* GLDebugSeverityToStr(const GLenum severity);

// Converts the boolean value into a GLboolean value.
GLboolean GLBoolean(bool value);

// Reads major/minor version from the string by glGetString(GL_VERSION), used for GL 2.x context creation.
bool GLParseVersionString(const GLubyte* s, GLint& major, GLint& minor);

// Returns the GL profile version as a single number, e.g. 450 for OpenGL 4.5.
int GLGetVersion();

// Traps program execution reporting a call to an unsupported OpenGL procedure.
[[noreturn]]
void ErrUnsupportedGLProc(const char* name);


} // /namespace LLGL


#endif



// ================================================================================
