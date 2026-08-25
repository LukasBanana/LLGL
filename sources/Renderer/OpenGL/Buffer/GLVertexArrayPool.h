/*
 * GLVertexArrayPool.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_VERTEX_ARRAY_POOL_H
#define LLGL_GL_VERTEX_ARRAY_POOL_H


#include "../OpenGL.h"
#include <queue>


namespace LLGL
{


/*
Pool to allocate and share VAOs.
The primary goal is to simplify releasing VAOs since GLSharedContextVertexArray contains potentially multiple VAOs spread across multiple GL contexts
and the release should not iterate through all GL context as making them current is an expensive operation.
*/
class GLVertexArrayPool
{

    public:

        #if LLGL_DEBUG
        struct Diagnostics
        {
            int alive       = 0; // Number of VAOs alive. This should be zero after GLRenderSystem is destroyed or else there are leaks.
            int maxAlive    = 0; // Maximum number of VAOs that were ever alive at one point.
            int requested   = 0; // Number of VAOs that have been requested.
            int destroyed   = 0; // Number of VAOs that have been destroyed.
        };
        #endif

    public:

        GLVertexArrayPool() = default;

        GLVertexArrayPool(const GLVertexArrayPool&) = delete;
        GLVertexArrayPool& operator = (const GLVertexArrayPool&) = delete;

        // Allocates a new VAO by using one from the existing pool or creating new ones.
        GLuint Allocate();

        // Releases the specified VAO, making it available for the next call to AllocateVertexArray() or being destroyed in Purge().
        // This function can be called on any GL context that is current, while Allocate() and Purge() must only be called when their parent GL context is current.
        void ReleaseOnAnyContext(GLuint vao);

        // Destroys all released VAOs by invoking glDeleteVertexArrays() on them.
        void Purge();

        #if LLGL_DEBUG
        // Returns the VAO pool diagnostics across all contexts. This is only active in a debug build.
        static const Diagnostics& GetDiagnostics();
        #endif

    private:

        std::queue<GLuint> pooledVAOs_;

};


} // /namespace LLGL


#endif



// ================================================================================
