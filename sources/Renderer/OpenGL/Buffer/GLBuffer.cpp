/*
 * GLBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLBuffer.h"
#include "../../GLCommon/GLTypes.h"
#include "../../GLCommon/GLExtensionRegistry.h"
#include "../Ext/GLExtensions.h"


namespace LLGL
{


GLBuffer::GLBuffer(const BufferType type) :
    Buffer { type }
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* Creates a new GL buffer object and binds it to an unspecified target */
        glCreateBuffers(1, &id_);
    }
    else
    #endif
    {
        /* Creates a new GL buffer object (must be bound to a target before it can be used) */
        glGenBuffers(1, &id_);
    }
}

GLBuffer::~GLBuffer()
{
    glDeleteBuffers(1, &id_);
}

void* GLBuffer::MapBuffer(GLenum access)
{
    #ifdef LLGL_GL_OPENGLES
    //TODO: move this into "Renderer/OpenGLES2/Buffer/GLES2Buffer.cpp"
    return glMapBufferOES(GetTarget(), access);
    #else
    return glMapBuffer(GetTarget(), access);
    #endif
}

GLboolean GLBuffer::UnmapBuffer()
{
    #ifdef LLGL_GL_OPENGLES
    //TODO: move this into "Renderer/OpenGLES2/Buffer/GLES2Buffer.cpp"
    return glUnmapBufferOES(GetTarget());
    #else
    return glUnmapBuffer(GetTarget());
    #endif
}


/*
 * ======= Private: =======
 */

GLenum GLBuffer::GetTarget() const
{
    return GLTypes::Map(GetType());
}


} // /namespace LLGL



// ================================================================================
