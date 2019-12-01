/*
 * GLESExtensionsDecl.inl
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

// THIS FILE MUST NOT HAVE A HEADER GUARD

/*
All OpenGLES extension functions are declared here.
Depending on the following macros being defined, the respective implementation is enabled:
- LLGL_DEF_GLES_PROXY_PROCS: defines the proxy functions for potentially unsupported GL extensions
- LLGL_DECL_GLES_PROXY_PROCS: declares the proxy functions for potentially unsupported GL extensions
- LLGL_DEF_GLES_EXT_PROCS: defines the global function pointer for GL extensions
- None: declares the global function pointer for GL extensions
*/


#if defined LLGL_DEF_GLES_PROXY_PROCS

#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    RTYPE APIENTRY Proxy_##NAME ARGS            \
    {                                           \
        ErrUnsupportedGLProc(#NAME);            \
    }

#elif defined LLGL_DECL_GLES_PROXY_PROCS

#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    RTYPE APIENTRY Proxy_##NAME ARGS

#elif defined LLGL_DEF_GLES_EXT_PROCS

#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    PFNTYPE NAME = nullptr

#else

#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    extern PFNTYPE NAME

#endif

/* GL_OES_tessellation_shader */

//DECL_GLPROC(PFNGLPATCHPARAMETERIPROC,                               glPatchParameteriOES,                           void,           (GLenum, GLint));

/* GL_ARB_compute_shader */

//DECL_GLPROC(PFNGLDISPATCHCOMPUTEPROC,                               glDispatchCompute,                              void,           (GLuint, GLuint, GLuint));
//DECL_GLPROC(PFNGLDISPATCHCOMPUTEINDIRECTPROC,                       glDispatchComputeIndirect,                      void,           (GLintptr));

#undef DECL_GLPROC



// ================================================================================
