/*
 * GLBufferWithVAO.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_BUFFER_WITH_VAO_H
#define LLGL_GL_BUFFER_WITH_VAO_H


#include "GLBuffer.h"
#include "GLVertexArrayObject.h"
#include "GLSharedContextVertexArray.h"
#include <LLGL/Container/ArrayView.h>


namespace LLGL
{


class GLBufferWithVAO : public GLBuffer
{

    public:

        GLBufferWithVAO(const BufferDescriptor& bufferDesc);

    private:

        //TODO: some form of VAO caching

};


} // /namespace LLGL


#endif



// ================================================================================
