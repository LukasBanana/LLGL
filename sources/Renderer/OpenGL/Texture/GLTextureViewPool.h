/*
 * GLTextureViewPool.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_TEXTURE_VIEW_POOL_H
#define LLGL_GL_TEXTURE_VIEW_POOL_H


#include <LLGL/TextureFlags.h>
#include <cstdint>
#include <vector>
#include "../OpenGL.h"
#include "../../TextureUtils.h"


namespace LLGL
{


// Class to manage create/reuse/delete of GL texture views; used by <GLResourceHeap>
class GLTextureViewPool
{

    public:

        // Returns the instance of this singleton.
        static GLTextureViewPool& Get();

    public:

        GLTextureViewPool(const GLTextureViewPool&) = delete;
        GLTextureViewPool& operator = (const GLTextureViewPool&) = delete;

        GLTextureViewPool(GLTextureViewPool&&) = delete;
        GLTextureViewPool& operator = (GLTextureViewPool&&) = delete;

        ~GLTextureViewPool();

        // Releases all resources for this singleton class.
        void Clear();

        /*
        Returns the ID of a GL texture view for the specified source texture and descriptor,
        or 0 if the extension "GL_ARB_texture_view" is not supported.
        */
        GLuint CreateTextureView(GLuint sourceTexID, const TextureViewDescriptor& textureViewDesc, bool restoreBoundTexture = true);

        // Release the texture view that was created with CreateTextureView.
        void ReleaseTextureView(GLuint texID);

        /*
        Notifes the texture view pool that the specified source texture was released.
        Tthis will also release all texture views derived from the specified texture.
        */
        void NotifyTextureRelease(GLuint sourceTexID);

    private:

        GLTextureViewPool() = default;

    private:

        // Structure that stores a GL texture that was generated with 'glTextureView'; managed by <GLTextureViewPool>
        struct GLTextureView
        {
            GLuint              texID        = 0;
            GLuint              sourceTexID  = 0;
            GLuint              refCount     = 0;
            CompressedTexView   view;
        };

        // Compares the two texture views in a strict-weak-order (SWO).
        static int CompareTextureViewSWO(const GLTextureView& lhs, const GLTextureView& rhs);

        // Creats a new GL texture view and stores it in the specified view entry.
        GLuint CreateGLTextureView(GLTextureView& texView, const TextureViewDescriptor& textureViewDesc, bool isSharedTex, bool restoreBoundTexture);

        // Deletes the specified GL texture view.
        void DeleteGLTextureView(GLTextureView& texView);

        // Assignes the specified texture view a new GL texture ID and reclaims it as a reusable entry.
        void RetainSharedGLTextureView(GLTextureView& texView, GLuint texID);

        // Release the specified GL texture object and marks the entry as reusable.
        void ReleaseSharedGLTextureView(GLTextureView& texView);

        // Removes all empty entries from the texture view list.
        void FlushReusableTextureViews();

    private:

        // Container of all managed texture views.
        std::vector<GLTextureView>  textureViews_;

        // Number of textures that are already freed, but not removed from the texture view array yet.
        std::size_t                 numReusableEntries_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
