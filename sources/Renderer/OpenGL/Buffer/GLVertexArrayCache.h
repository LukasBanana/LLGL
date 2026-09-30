/*
 * GLVertexArrayCache.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_VERTEX_ARRAY_CACHE_H
#define LLGL_GL_VERTEX_ARRAY_CACHE_H


#include <LLGL/VertexAttribute.h>
#include <LLGL/Container/SmallVector.h>
#include "GLSharedContextVertexArray.h"
#include "GLBufferInputLayout.h"
#include <vector>
#include <mutex>


namespace LLGL
{


// Singleton for caching VAOs for combinations between vertex buffers and vertex input layouts.
// These VAOs are shared across contexts (GLSharedContextVertexArray).
class GLVertexArrayCache
{

    public:

        GLVertexArrayCache(const GLVertexArrayCache&) = delete;
        GLVertexArrayCache& operator = (const GLVertexArrayCache&) = delete;

        // Returns the instance of this cache.
        static GLVertexArrayCache& Get();

        // Releaes all VAOs and resets all cache entries.
        void Clear();

        /*
        Returns a vertex array object (VAO) for the specified vertex input and buffer input layouts.
        This function either returns an existing VAO or creates a new one if the specified combination doesn't have an allocated VAO yet.
        The lookup uses the hash of both input layouts. The returned VAOs are shared across one or more GL contexts.
        NOTE:
          This class must interpret `GLVertexAttribute::buffer` as a zero-based index, not the actual GL buffer ID!
          This function converts that index to the respective GL buffer ID from the `bufferInputLayout` parameter/
        */
        GLSharedContextVertexArray* FindOrMakeVertexArray(const GLVertexInputLayout& vertexInputLayout, const GLBufferInputLayout& bufferInputLayout);

        /*
        Notifies this cache that the specified GL buffer has been released.
        This will destroy all cached VAOs that include the specified buffer.
        */
        void NotifyBufferRelease(const GLBuffer& buffer);

    private:

        struct VertexBufferBinding
        {
            GLVertexInputLayout             vertexInputLayout;
            GLBufferInputLayout             bufferInputLayout;
            GLSharedContextVertexArrayPtr   vertexArray;
        };

    private:

        GLVertexArrayCache() = default;

    private:

        std::mutex                          mutex_;
        std::vector<VertexBufferBinding>    vertexBindings_;

};


} // /namespace LLGL


#endif



// ================================================================================
