/*
 * GLCompatExtensionMapping.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_COMPAT_EXTENSION_MAPPING_H
#define LLGL_GL_COMPAT_EXTENSION_MAPPING_H


#include "../../OpenGL.h"


#if GL_EXT_framebuffer_object

#define glGenFramebuffers                        glGenFramebuffersEXT
#define glDeleteFramebuffers                     glDeleteFramebuffersEXT
#define glBindFramebuffer                        glBindFramebufferEXT
#define glBindRenderbuffer                       glBindRenderbufferEXT
#define glBlitFramebuffer                        glBlitFramebufferEXT
#define glFramebufferTexture1D                   glFramebufferTexture1DEXT
#define glFramebufferTexture2D                   glFramebufferTexture2DEXT
#define glFramebufferTexture3D                   glFramebufferTexture3DEXT
#define glFramebufferRenderbuffer                glFramebufferRenderbufferEXT
#define glCheckFramebufferStatus                 glCheckFramebufferStatusEXT
#define glGenRenderbuffers                       glGenRenderbuffersEXT
#define glDeleteRenderbuffers                    glDeleteRenderbuffersEXT
#define glRenderbufferStorage                    glRenderbufferStorageEXT
#define glRenderbufferStorageMultisample         glRenderbufferStorageMultisampleEXT
#define glGetRenderbufferParameteriv             glGetRenderbufferParameterivEXT
#define glGetFramebufferAttachmentParameteriv    glGetFramebufferAttachmentParameterivEXT
#define glGenerateMipmap                         glGenerateMipmapEXT

#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER                  GL_FRAMEBUFFER_EXT
#endif

#ifndef GL_READ_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER             GL_READ_FRAMEBUFFER_EXT
#endif

#ifndef GL_DRAW_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER             GL_DRAW_FRAMEBUFFER_EXT
#endif

#ifndef GL_FRAMEBUFFER_COMPLETE
#define GL_FRAMEBUFFER_COMPLETE         GL_FRAMEBUFFER_COMPLETE_EXT
#endif

#ifndef GL_RENDERBUFFER_WIDTH
#define GL_RENDERBUFFER_WIDTH           GL_RENDERBUFFER_WIDTH_EXT
#endif

#ifndef GL_RENDERBUFFER_HEIGHT
#define GL_RENDERBUFFER_HEIGHT          GL_RENDERBUFFER_HEIGHT_EXT
#endif

#ifndef GL_RENDERBUFFER_INTERNAL_FORMAT
#define GL_RENDERBUFFER_INTERNAL_FORMAT GL_RENDERBUFFER_INTERNAL_FORMAT_EXT
#endif

#ifndef GL_DEPTH_ATTACHMENT
#define GL_DEPTH_ATTACHMENT             GL_DEPTH_ATTACHMENT_EXT
#endif

#ifndef GL_STENCIL_ATTACHMENT
#define GL_STENCIL_ATTACHMENT           GL_STENCIL_ATTACHMENT_EXT
#endif

#ifndef GL_COLOR_ATTACHMENT0
#define GL_COLOR_ATTACHMENT0            GL_COLOR_ATTACHMENT0_EXT
#endif

#ifndef GL_COLOR_ATTACHMENT1
#define GL_COLOR_ATTACHMENT1            GL_COLOR_ATTACHMENT1_EXT
#endif

#ifndef GL_COLOR_ATTACHMENT2
#define GL_COLOR_ATTACHMENT2            GL_COLOR_ATTACHMENT2_EXT
#endif

#ifndef GL_COLOR_ATTACHMENT3
#define GL_COLOR_ATTACHMENT3            GL_COLOR_ATTACHMENT3_EXT
#endif

#ifndef GL_COLOR_ATTACHMENT4
#define GL_COLOR_ATTACHMENT4            GL_COLOR_ATTACHMENT4_EXT
#endif

#ifndef GL_COLOR_ATTACHMENT5
#define GL_COLOR_ATTACHMENT5            GL_COLOR_ATTACHMENT5_EXT
#endif

#ifndef GL_COLOR_ATTACHMENT6
#define GL_COLOR_ATTACHMENT6            GL_COLOR_ATTACHMENT6_EXT
#endif

#ifndef GL_COLOR_ATTACHMENT7
#define GL_COLOR_ATTACHMENT7            GL_COLOR_ATTACHMENT7_EXT
#endif

#ifndef GL_COLOR_ATTACHMENT8
#define GL_COLOR_ATTACHMENT8            GL_COLOR_ATTACHMENT8_EXT
#endif

#ifndef GL_RENDERBUFFER
#define GL_RENDERBUFFER                 GL_RENDERBUFFER_EXT
#endif

#endif // /GL_EXT_framebuffer_object


#endif



// ================================================================================
