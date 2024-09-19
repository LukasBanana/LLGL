/*
 * GL3PlusSharedContextVertexArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL3PLUS_SHARED_CONTEXT_VERTEX_ARRAY_H
#define LLGL_GL3PLUS_SHARED_CONTEXT_VERTEX_ARRAY_H


#include <LLGL/VertexAttribute.h>
#include <LLGL/Container/ArrayView.h>
#include "GLVertexAttribute.h"
#include "GLVertexArrayObject.h"
#include <vector>


namespace LLGL
{


class GLStateManager;

// This class manages a vertex-array-object (VAO) across one or more GL contexts.
class GL3PlusSharedContextVertexArray
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

        struct GLContextVAO
        {
            GLVertexArrayObject vao;
            bool                isObjectLabelDirty = false;

            void SetObjectLabel(const char* label);
        };

    private:

        // Returns the VAO for the current GL context and creates it on demand.
        GLVertexArrayObject& GetVAOForCurrentContext();

    private:

        std::vector<GLVertexAttribute>  attribs_;
        std::vector<GLContextVAO>       contextDependentVAOs_;
        std::string                     debugName_;

};


} // /namespace LLGL


#endif



// ================================================================================
