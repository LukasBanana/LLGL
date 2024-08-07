/*
 * GLSharedContextVertexArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL2X_VERTEX_ARRAY_H
#define LLGL_GL2X_VERTEX_ARRAY_H


#include <LLGL/VertexAttribute.h>
#include <LLGL/Container/ArrayView.h>
#include "GLVertexArrayObject.h"
#include <vector>


namespace LLGL
{


class GLStateManager;

// This class emulates the Vertex-Array-Object (VAO) functionality, for GL 2.x.
class GLSharedContextVertexArray
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

        void BuildVertexLayoutForGL3Plus(GLuint bufferID, const ArrayView<VertexAttribute>& attributes);
        void BindForGL3Plus(GLStateManager& stateMngr);

        #ifdef LLGL_GL_ENABLE_OPENGL2X

        void BuildVertexLayoutForGL2X(GLuint bufferID, const ArrayView<VertexAttribute>& attributes);
        void BindForGL2X(GLStateManager& stateMngr);
        void FinalizeForGL2X();

        #endif // /LLGL_GL_ENABLE_OPENGL2X

    private:

        std::vector<GLVertexAttribute>      attribs_;
        std::vector<GLContextVAO>           contextDependentVAOs_;

        #ifdef LLGL_GL_ENABLE_OPENGL2X
        GLuint                              attribIndexEnd_ = 0;
        #endif

        std::string                         debugName_;

};


} // /namespace LLGL


#endif



// ================================================================================
