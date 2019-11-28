/*
 * GLProfile.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_PROFILE_H
#define LLGL_GL_PROFILE_H


#include "OpenGL.h"


namespace LLGL
{

// Wrapper namespace to abstract OpenGL and OpenGLES API calls
namespace GLProfile
{


// Returns the OpenGL API name, e.g. "OpenGL" or "OpenGLES".
const char* GetAPIName();

// Returns the OpenGL shading language name, e.g. "GLSL" or "ESSL".
const char* GetShadingLanguageName();


} // /namespace GLProfile

} // /namespace LLGL


#endif



// ================================================================================
