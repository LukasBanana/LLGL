/*
 * GL2XSharedContextVertexArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL2X_SHARED_CONTEXT_VERTEX_ARRAY_H
#define LLGL_GL2X_SHARED_CONTEXT_VERTEX_ARRAY_H


#include <LLGL/VertexAttribute.h>
#include <LLGL/Container/ArrayView.h>
#include "GLVertexAttribute.h"
#include "GLVertexArrayObject.h"
#include <vector>


namespace LLGL
{


class GLStateManager;

// This class emulates the Vertex-Array-Object (VAO) functionality, for GL 2.x.
class GL2XSharedContextVertexArray
{

    public:

        // Stores the vertex attributes for later use via glVertexAttrib*Pointer() functions.
        void BuildVertexLayout(GLuint bufferID, const ArrayView<VertexAttribute>& attributes);

        // Finalize the vertex array.
        void Finalize();

        // Binds this vertex array.
        void Bind(GLStateManager& stateMngr);

        // Sets the debug label for all VAOs.
        void SetDebugName(const char* name);

    private:

        std::vector<GLVertexAttribute>  attribs_;
        GLuint                          attribIndexEnd_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
