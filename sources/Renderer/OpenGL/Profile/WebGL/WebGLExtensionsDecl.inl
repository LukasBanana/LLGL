/*
 * WebGLExtensionsDecl.inl
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

// THIS FILE MUST NOT HAVE A HEADER GUARD


#if !GL_GLEXT_PROTOTYPES

#ifndef DECL_GLPROC
#error Missing definition of macro DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS)
#endif

/* GL_OES_draw_buffers_indexed */

DECL_GLPROC(OES_draw_buffers_indexed, PFNGLENABLEIOESPROC,                  glEnableiOES,                   void,       (GLenum target, GLuint index), (target, index));
DECL_GLPROC(OES_draw_buffers_indexed, PFNGLDISABLEIOESPROC,                 glDisableiOES,                  void,       (GLenum target, GLuint index), (target, index));
DECL_GLPROC(OES_draw_buffers_indexed, PFNGLBLENDEQUATIONIOESPROC,           glBlendEquationiOES,            void,       (GLuint buf, GLenum mode), (buf, mode));
DECL_GLPROC(OES_draw_buffers_indexed, PFNGLBLENDEQUATIONSEPARATEIOESPROC,   glBlendEquationSeparateiOES,    void,       (GLuint buf, GLenum modeRGB, GLenum modeAlpha), (buf, modeRGB, modeAlpha));
DECL_GLPROC(OES_draw_buffers_indexed, PFNGLBLENDFUNCIOESPROC,               glBlendFunciOES,                void,       (GLuint buf, GLenum src, GLenum dst), (buf, src, dst));
DECL_GLPROC(OES_draw_buffers_indexed, PFNGLBLENDFUNCSEPARATEIOESPROC,       glBlendFuncSeparateiOES,        void,       (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha), (buf, srcRGB, dstRGB, srcAlpha, dstAlpha));
DECL_GLPROC(OES_draw_buffers_indexed, PFNGLCOLORMASKIOESPROC,               glColorMaskiOES,                void,       (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a), (index, r, g, b, a));
//DECL_GLPROC(OES_draw_buffers_indexed, PFNGLISENABLEDIOESPROC,               glIsEnablediOES,                GLboolean,  (GLenum target, GLuint index), (target, index));

#endif // /!GL_GLEXT_PROTOTYPES



// ================================================================================
