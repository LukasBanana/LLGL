/*
 * GLFramebufferCapture.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLFramebufferCapture.h"
#include "GLTexture.h"
#include "../RenderState/GLStateManager.h"
#include "../Profile/GLProfile.h"
#include "../GLTypes.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../../CheckedCast.h"


namespace LLGL
{


/*
 * GLIntermediateTexture structure
 */

GLIntermediateTexture::~GLIntermediateTexture()
{
    ReleaseTexture();
}

void GLIntermediateTexture::CreateTexture()
{
    if (texID == 0)
        glGenTextures(1, &texID);
}

void GLIntermediateTexture::ReleaseTexture()
{
    if (texID != 0)
    {
        glDeleteTextures(1, &texID);
        texID = 0;
    }
}


/*
 * GLFramebufferCapture class
 */

GLFramebufferCapture& GLFramebufferCapture::Get()
{
    static GLFramebufferCapture instance;
    return instance;
}

void GLFramebufferCapture::Clear()
{
    blitTextureFBOPair_.ReleaseFBOs();
    intermediateTex_.ReleaseTexture();
}

static void BlitFramebufferNearestFlippedYAxis(GLint x, GLint y, GLint width, GLint height, GLbitfield bitmask)
{
    glBlitFramebuffer(0, 0, width, height, x, y + height, x + width, y, bitmask, GL_NEAREST);
}

void GLFramebufferCapture::CaptureFramebuffer(
    GLStateManager& stateMngr,
    GLTexture&      textureGL,
    GLint           dstLevel,
    const Offset3D& dstOffset,
    const Offset2D& srcOffset,
    const Extent2D& extent)
{
    if (textureGL.IsRenderbuffer())
        return /*GL_INVALID_VALUE*/;

    const TextureType       type            = textureGL.GetType();
    const FormatAttributes& formatAttribs   = GetFormatAttribs(textureGL.GetFormat());
    const bool              hasDepth        = ((formatAttribs.flags & FormatFlags::HasDepth) != 0);
    const bool              hasStencil      = ((formatAttribs.flags & FormatFlags::HasStencil) != 0);
    const bool              isDepthStencil  = (hasDepth || hasStencil);

    const GLTextureTarget   target          = GLStateManager::GetTextureTarget(type);
    const GLenum            targetGL        = GLTypes::Map(type);
    const GLsizei           width           = static_cast<GLsizei>(extent.width);
    const GLsizei           height          = static_cast<GLsizei>(extent.height);
    const GLenum            attachment      = (isDepthStencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_COLOR_ATTACHMENT0);
    const GLbitfield        bitmask         = (isDepthStencil ? GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT : GL_COLOR_BUFFER_BIT);

    /* Translate framebuffer offset to lower-left coordinate system */
    const GLint             screenPosX      = srcOffset.x;
    const GLint             screenPosY      = stateMngr.GetFramebufferHeight() - height - srcOffset.y;

    /* Create intermediate texture and FBOs */
    intermediateTex_.CreateTexture();
    blitTextureFBOPair_.CreateFBOs();

    /* Copy framebuffer into intermediate texture */
    stateMngr.PushBoundTexture(target);
    {
        stateMngr.BindTexture(target, intermediateTex_.texID);

        #if !LLGL_GL_ENABLE_OPENGL2X
        if (hasStencil)
        {
            /* Allocate storage for intermediate texture with depth-stencil format (only supported in GL 3+) */
            glTexImage2D(targetGL, 0, textureGL.GetGLInternalFormat(), width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, nullptr);
        }
        else
        #endif // /!LLGL_GL_ENABLE_OPENGL2X
        if (hasDepth)
        {
            /* Allocate storage for intermediate texture with depth-component only format */
            glTexImage2D(targetGL, 0, textureGL.GetGLInternalFormat(), width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        }
        else
        {
            /* Allocate storage for intermediate texture with color format */
            glTexImage2D(targetGL, 0, textureGL.GetGLInternalFormat(), width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }

        glCopyTexSubImage2D(targetGL, 0, 0, 0, screenPosX, screenPosY, width, height);
    }
    stateMngr.PopBoundTexture();

    /* Blit intermediate texture into destination texture with flipped Y-axis to accommodate OpenGL coordinate system */
    stateMngr.PushBoundFramebuffer(GLFramebufferTarget::ReadFramebuffer);
    stateMngr.PushBoundFramebuffer(GLFramebufferTarget::DrawFramebuffer);
    {
        /* Bind read framebuffer for intermediate texture and draw framebuffer for destination texture */
        stateMngr.BindFramebuffer(GLFramebufferTarget::ReadFramebuffer, blitTextureFBOPair_.fbos[0]);
        stateMngr.BindFramebuffer(GLFramebufferTarget::DrawFramebuffer, blitTextureFBOPair_.fbos[1]);

        GLProfile::FramebufferTexture2D(GL_READ_FRAMEBUFFER, attachment, targetGL, intermediateTex_.texID, 0);
        GLFramebuffer::AttachTexture(textureGL, attachment, dstLevel, dstOffset.z, GL_DRAW_FRAMEBUFFER);

        BlitFramebufferNearestFlippedYAxis(dstOffset.x, dstOffset.y, width, height, bitmask);
    }
    stateMngr.PopBoundFramebuffer();
    stateMngr.PopBoundFramebuffer();
}


} // /namespace LLGL



// ================================================================================
