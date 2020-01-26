/*
 * GLTextureViewPool.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTextureViewPool.h"
#include "GLTexture.h"
#include "../RenderState/GLStateManager.h"
#include "../GLProfile.h"
#include "../GLTypes.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../../CheckedCast.h"
#include "../../../Core/ContainerUtils.h"
#include "../../../Core/Helper.h"
#include "../../../Core/HelperMacros.h"
#include <algorithm>


namespace LLGL
{


static const std::size_t g_maxNumReusableTextureViews = 16;

GLTextureViewPool::~GLTextureViewPool()
{
    Clear();
}

GLTextureViewPool& GLTextureViewPool::Get()
{
    static GLTextureViewPool instance;
    return instance;
}

void GLTextureViewPool::Clear()
{
    /* Delete all texture view GL objects and clear container */
    for (const auto& texView : textureViews_)
    {
        if (texView.texID != 0)
            glDeleteTextures(1, &(texView.texID));
    }
    textureViews_.clear();
    numReusableEntries_ = 0;
}

GLuint GLTextureViewPool::CreateTextureView(GLuint sourceTexID, const TextureViewDescriptor& textureViewDesc, bool restoreBoundTexture)
{
    #ifdef GL_ARB_texture_view

    if (!HasExtension(GLExt::ARB_texture_view))
        return 0;

    /* Compress texture view descriptor for faster comparison and sorting */
    GLTextureView texView;
    {
        texView.sourceTexID = sourceTexID;
    }
    CompressTextureViewDesc(texView.view, textureViewDesc);

    /* Try to find texture view with same parameters */
    std::size_t insertionIndex = 0;
    auto* sharedTexView = Utils::FindInSortedArray<GLTextureView>(
        textureViews_.data(),
        textureViews_.size(),
        [&texView](const GLTextureView& rhs)
        {
            return GLTextureViewPool::CompareTextureViewSWO(texView, rhs);
        },
        &insertionIndex
    );

    if (sharedTexView != nullptr)
    {
        /* Create a shared GL texture view from the descriptor */
        return CreateGLTextureView(*sharedTexView, textureViewDesc, true, restoreBoundTexture);
    }
    else
    {
        /* Create new GL texture view and store it with insertion sort */
        GLuint texID = CreateGLTextureView(texView, textureViewDesc, false, restoreBoundTexture);
        textureViews_.insert(textureViews_.begin() + insertionIndex, texView);
        return texID;
    }

    #else

    return 0;

    #endif
}

void GLTextureViewPool::ReleaseTextureView(GLuint texID)
{
    /* Try to find texture by GL texture ID only */
    std::size_t insertionIndex = 0;
    auto* sharedTexView = Utils::FindInSortedArray<GLTextureView>(
        textureViews_.data(),
        textureViews_.size(),
        [texID](const GLTextureView& rhs)
        {
            LLGL_COMPARE_SEPARATE_MEMBERS_SWO(texID, rhs.texID);
            return 0;
        },
        &insertionIndex
    );

    if (sharedTexView != nullptr)
    {
        /* Delete GL texture view if the reference counter reaches 0 */
        ReleaseSharedGLTextureView(*sharedTexView);

        /* Remove unused entries in the array after a given amount has been freed */
        if (numReusableEntries_ > g_maxNumReusableTextureViews)
            FlushReusableTextureViews();
    }
}

void GLTextureViewPool::NotifyTextureRelease(GLuint sourceTexID)
{
    /* Move all objects that are about to be removed at the end of the list using 'std::remove' */
    auto it = std::remove_if(
        textureViews_.begin(),
        textureViews_.end(),
        [sourceTexID](const GLTextureView& entry)
        {
            return (entry.texID == 0 || entry.sourceTexID == sourceTexID);
        }
    );

    /* Iterate over all elements to be deleted */
    for (; it != textureViews_.end(); ++it)
        DeleteGLTextureView(*it);

    /* Finally remove the entries from the list */
    textureViews_.erase(it, textureViews_.end());

    /* Reset reusable entries, since we just deleted all unused entries */
    numReusableEntries_ = 0;
}


/*
 * ======= Private: =======
 */

int GLTextureViewPool::CompareTextureViewSWO(const GLTextureView& lhs, const GLTextureView& rhs)
{
    LLGL_COMPARE_MEMBER_SWO( sourceTexID );
    return CompareCompressedTexViewSWO(lhs.view, rhs.view);
}

#ifdef GL_ARB_texture_view

static void InitializeTextureViewSwizzle(GLuint texID, const GLTextureTarget target, const TextureViewDescriptor& textureViewDesc)
{
    GLStateManager::Get().BindTexture(target, texID);
    GLTexture::TexParameterSwizzle(textureViewDesc.type, textureViewDesc.format, textureViewDesc.swizzle);
}

#endif // /GL_ARB_texture_view

static GLuint GenGLTextureView(GLuint sourceTexID, const TextureViewDescriptor& textureViewDesc, bool restoreBoundTexture)
{
    GLuint texID = 0;

    #ifdef GL_ARB_texture_view

    if (HasExtension(GLExt::ARB_texture_view))
    {
        /* Generate and initialize texture with texture-view description */
        glGenTextures(1, &texID);
        glTextureView(
            texID,
            GLTypes::Map(textureViewDesc.type),
            sourceTexID,
            GLTypes::Map(textureViewDesc.format),
            textureViewDesc.subresource.baseMipLevel,
            textureViewDesc.subresource.numMipLevels,
            textureViewDesc.subresource.baseArrayLayer,
            textureViewDesc.subresource.numArrayLayers
        );

        /* Initialize texture swizzle */
        const auto target = GLStateManager::GetTextureTarget(textureViewDesc.type);
        if (restoreBoundTexture)
        {
            /* Initialize texture view with swizzle parameters and store/restore bound texture slot */
            GLStateManager::Get().PushBoundTexture(target);
            {
                InitializeTextureViewSwizzle(texID, target, textureViewDesc);
            }
            GLStateManager::Get().PopBoundTexture();
        }
        else
        {
            /* Initialize texture view with swizzle parameters */
            InitializeTextureViewSwizzle(texID, target, textureViewDesc);
        }
    }

    #endif // /GL_ARB_texture_view

    return texID;
}

GLuint GLTextureViewPool::CreateGLTextureView(GLTextureView& texView, const TextureViewDescriptor& textureViewDesc, bool isSharedTex, bool restoreBoundTexture)
{
    if (isSharedTex)
    {
        if (texView.texID == 0)
        {
            /* Create new GL texture view if ID is invalid */
            GLuint texID = GenGLTextureView(texView.sourceTexID, textureViewDesc, restoreBoundTexture);
            RetainSharedGLTextureView(texView, texID);
        }
    }
    else
    {
        /* Create a new GL texture view and initialize reference counter */
        texView.texID = GenGLTextureView(texView.sourceTexID, textureViewDesc, restoreBoundTexture);
    }

    /* Increment reference counter for the shared texture view */
    texView.refCount++;

    return texView.texID;
}

// Uncompresses the specified 4-bit texture type to a 'GLTextureTarget' enum entry.
static GLTextureTarget UncompressGLTextureTarget(std::uint32_t type)
{
    return GLStateManager::GetTextureTarget(static_cast<TextureType>(type));
}

void GLTextureViewPool::DeleteGLTextureView(GLTextureView& texView)
{
    GLStateManager::Get().DeleteTexture(texView.texID, UncompressGLTextureTarget(texView.view.type));
}

void GLTextureViewPool::RetainSharedGLTextureView(GLTextureView& texView, GLuint texID)
{
    if (texView.texID == 0)
    {
        /* If this is shared texture view, reclaim one of the reusable entries */
        texView.texID = texID;
        if (numReusableEntries_ > 0)
            --numReusableEntries_;
    }
}

void GLTextureViewPool::ReleaseSharedGLTextureView(GLTextureView& texView)
{
    if (texView.refCount > 0)
    {
        texView.refCount--;
        if (texView.refCount == 0)
        {
            DeleteGLTextureView(texView);
            ++numReusableEntries_;
        }
    }
}

void GLTextureViewPool::FlushReusableTextureViews()
{
    if (numReusableEntries_ > 0)
    {
        RemoveAllFromListIf(
            textureViews_,
            [](const GLTextureView& entry)
            {
                return (entry.texID == 0);
            }
        );
        numReusableEntries_ = 0;
    }
}


} // /namespace LLGL



// ================================================================================
