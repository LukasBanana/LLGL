/*
 * GLObjectUtils.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_OBJECT_UTILS_H
#define LLGL_GL_OBJECT_UTILS_H


#include "OpenGL.h"
#include <cstdint>


namespace LLGL
{


/*
Sets the label for the specified GL object <name>, where <identifier> must one of the following values:
GL_BUFFER, GL_FRAMEBUFFER, GL_PROGRAM, GL_PROGRAM_PIPELINE, GL_QUERY, GL_RENDERBUFFER, GL_SAMPLER, GL_SHADER, GL_TEXTURE, GL_TRANSFORM_FEEDBACK, GL_VERTEX_ARRAY.
*/
void GLSetObjectLabel(GLenum identifier, GLuint name, const char* label);

// Sets the label for the specified GL object <name> with an altered name with a subscript (see GLSetObjectLabel).
void GLSetObjectLabelSubscript(GLenum identifier, GLuint name, const char* label, const char* subscript);

// Sets the label for the specified GL object <name> with an altered name with an indexed subscript (see GLSetObjectLabel).
void GLSetObjectLabelIndexed(GLenum identifier, GLuint name, const char* label, std::uint32_t idx);


} // /namespace LLGL


#endif



// ================================================================================
