/*
 * GLObjectUtils.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_OBJECT_UTILS_H
#define LLGL_GL_OBJECT_UTILS_H


#include "OpenGL.h"
#include <cstdint>


#ifndef GL_BUFFER
#define GL_BUFFER 0x82E0
#endif

#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER 0x8D40
#endif

#ifndef GL_PROGRAM
#define GL_PROGRAM 0x82E2
#endif

#ifndef GL_PROGRAM_PIPELINE
#define GL_PROGRAM_PIPELINE 0x82E4
#endif

#ifndef GL_QUERY
#define GL_QUERY 0x82E3
#endif

#ifndef GL_RENDERBUFFER
#define GL_RENDERBUFFER 0x8D41
#endif

#ifndef GL_SAMPLER
#define GL_SAMPLER 0x82E6
#endif

#ifndef GL_SHADER
#define GL_SHADER 0x82E1
#endif

#ifndef GL_TEXTURE
#define GL_TEXTURE 0x1702
#endif

#ifndef GL_TRANSFORM_FEEDBACK
#define GL_TRANSFORM_FEEDBACK 0x8E22
#endif

#ifndef GL_VERTEX_ARRAY
#define GL_VERTEX_ARRAY 0x8074
#endif


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
void GLSetObjectLabelIndexed(GLenum identifier, GLuint name, const char* label, std::uint32_t index);

// Sets the label for a GL sync object.
void GLSetObjectPtrLabel(void* ptr, const char* label);


} // /namespace LLGL


#endif



// ================================================================================
