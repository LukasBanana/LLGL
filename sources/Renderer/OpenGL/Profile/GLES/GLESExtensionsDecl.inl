/*
 * GLESExtensionsDecl.inl
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

// THIS FILE MUST NOT HAVE A HEADER GUARD


#if !GL_GLEXT_PROTOTYPES

#ifndef DECL_GLPROC
#error Missing definition of macro DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS)
#endif

#if GL_ES_VERSION_3_1

DECL_GLPROC(PFNGLDISPATCHCOMPUTEPROC,                   glDispatchCompute,                  void,       (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z));
DECL_GLPROC(PFNGLDISPATCHCOMPUTEINDIRECTPROC,           glDispatchComputeIndirect,          void,       (GLintptr indirect));
DECL_GLPROC(PFNGLDRAWARRAYSINDIRECTPROC,                glDrawArraysIndirect,               void,       (GLenum mode, const void *indirect));
DECL_GLPROC(PFNGLDRAWELEMENTSINDIRECTPROC,              glDrawElementsIndirect,             void,       (GLenum mode, GLenum type, const void *indirect));
DECL_GLPROC(PFNGLFRAMEBUFFERPARAMETERIPROC,             glFramebufferParameteri,            void,       (GLenum target, GLenum pname, GLint param));
DECL_GLPROC(PFNGLGETFRAMEBUFFERPARAMETERIVPROC,         glGetFramebufferParameteriv,        void,       (GLenum target, GLenum pname, GLint *params));
DECL_GLPROC(PFNGLGETPROGRAMINTERFACEIVPROC,             glGetProgramInterfaceiv,            void,       (GLuint program, GLenum programInterface, GLenum pname, GLint *params));
DECL_GLPROC(PFNGLGETPROGRAMRESOURCEINDEXPROC,           glGetProgramResourceIndex,          GLuint,     (GLuint program, GLenum programInterface, const GLchar *name));
DECL_GLPROC(PFNGLGETPROGRAMRESOURCENAMEPROC,            glGetProgramResourceName,           void,       (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name));
DECL_GLPROC(PFNGLGETPROGRAMRESOURCEIVPROC,              glGetProgramResourceiv,             void,       (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint* params));
DECL_GLPROC(PFNGLGETPROGRAMRESOURCELOCATIONPROC,        glGetProgramResourceLocation,       GLint,      (GLuint program, GLenum programInterface, const GLchar *name));
DECL_GLPROC(PFNGLUSEPROGRAMSTAGESPROC,                  glUseProgramStages,                 void,       (GLuint pipeline, GLbitfield stages, GLuint program));
DECL_GLPROC(PFNGLACTIVESHADERPROGRAMPROC,               glActiveShaderProgram,              void,       (GLuint pipeline, GLuint program));
DECL_GLPROC(PFNGLCREATESHADERPROGRAMVPROC,              glCreateShaderProgramv,             GLuint,     (GLenum type, GLsizei count, const GLchar *const*strings));
DECL_GLPROC(PFNGLBINDPROGRAMPIPELINEPROC,               glBindProgramPipeline,              void,       (GLuint pipeline));
DECL_GLPROC(PFNGLDELETEPROGRAMPIPELINESPROC,            glDeleteProgramPipelines,           void,       (GLsizei n, const GLuint *pipelines));
DECL_GLPROC(PFNGLGENPROGRAMPIPELINESPROC,               glGenProgramPipelines,              void,       (GLsizei n, GLuint *pipelines));
DECL_GLPROC(PFNGLISPROGRAMPIPELINEPROC,                 glIsProgramPipeline,                GLboolean,  (GLuint pipeline));
DECL_GLPROC(PFNGLGETPROGRAMPIPELINEIVPROC,              glGetProgramPipelineiv,             void,       (GLuint pipeline, GLenum pname, GLint *params));
DECL_GLPROC(PFNGLPROGRAMUNIFORM1IPROC,                  glProgramUniform1i,                 void,       (GLuint program, GLint location, GLint v0));
DECL_GLPROC(PFNGLPROGRAMUNIFORM2IPROC,                  glProgramUniform2i,                 void,       (GLuint program, GLint location, GLint v0, GLint v1));
DECL_GLPROC(PFNGLPROGRAMUNIFORM3IPROC,                  glProgramUniform3i,                 void,       (GLuint program, GLint location, GLint v0, GLint v1, GLint v2));
DECL_GLPROC(PFNGLPROGRAMUNIFORM4IPROC,                  glProgramUniform4i,                 void,       (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3));
DECL_GLPROC(PFNGLPROGRAMUNIFORM1UIPROC,                 glProgramUniform1ui,                void,       (GLuint program, GLint location, GLuint v0));
DECL_GLPROC(PFNGLPROGRAMUNIFORM2UIPROC,                 glProgramUniform2ui,                void,       (GLuint program, GLint location, GLuint v0, GLuint v1));
DECL_GLPROC(PFNGLPROGRAMUNIFORM3UIPROC,                 glProgramUniform3ui,                void,       (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2));
DECL_GLPROC(PFNGLPROGRAMUNIFORM4UIPROC,                 glProgramUniform4ui,                void,       (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3));
DECL_GLPROC(PFNGLPROGRAMUNIFORM1FPROC,                  glProgramUniform1f,                 void,       (GLuint program, GLint location, GLfloat v0));
DECL_GLPROC(PFNGLPROGRAMUNIFORM2FPROC,                  glProgramUniform2f,                 void,       (GLuint program, GLint location, GLfloat v0, GLfloat v1));
DECL_GLPROC(PFNGLPROGRAMUNIFORM3FPROC,                  glProgramUniform3f,                 void,       (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2));
DECL_GLPROC(PFNGLPROGRAMUNIFORM4FPROC,                  glProgramUniform4f,                 void,       (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3));
DECL_GLPROC(PFNGLPROGRAMUNIFORM1IVPROC,                 glProgramUniform1iv,                void,       (GLuint program, GLint location, GLsizei count, const GLint *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORM2IVPROC,                 glProgramUniform2iv,                void,       (GLuint program, GLint location, GLsizei count, const GLint *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORM3IVPROC,                 glProgramUniform3iv,                void,       (GLuint program, GLint location, GLsizei count, const GLint *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORM4IVPROC,                 glProgramUniform4iv,                void,       (GLuint program, GLint location, GLsizei count, const GLint *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORM1UIVPROC,                glProgramUniform1uiv,               void,       (GLuint program, GLint location, GLsizei count, const GLuint *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORM2UIVPROC,                glProgramUniform2uiv,               void,       (GLuint program, GLint location, GLsizei count, const GLuint *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORM3UIVPROC,                glProgramUniform3uiv,               void,       (GLuint program, GLint location, GLsizei count, const GLuint *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORM4UIVPROC,                glProgramUniform4uiv,               void,       (GLuint program, GLint location, GLsizei count, const GLuint *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORM1FVPROC,                 glProgramUniform1fv,                void,       (GLuint program, GLint location, GLsizei count, const GLfloat *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORM2FVPROC,                 glProgramUniform2fv,                void,       (GLuint program, GLint location, GLsizei count, const GLfloat *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORM3FVPROC,                 glProgramUniform3fv,                void,       (GLuint program, GLint location, GLsizei count, const GLfloat *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORM4FVPROC,                 glProgramUniform4fv,                void,       (GLuint program, GLint location, GLsizei count, const GLfloat *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORMMATRIX2FVPROC,           glProgramUniformMatrix2fv,          void,       (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORMMATRIX3FVPROC,           glProgramUniformMatrix3fv,          void,       (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORMMATRIX4FVPROC,           glProgramUniformMatrix4fv,          void,       (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC,         glProgramUniformMatrix2x3fv,        void,       (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC,         glProgramUniformMatrix3x2fv,        void,       (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC,         glProgramUniformMatrix2x4fv,        void,       (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC,         glProgramUniformMatrix4x2fv,        void,       (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC,         glProgramUniformMatrix3x4fv,        void,       (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value));
DECL_GLPROC(PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC,         glProgramUniformMatrix4x3fv,        void,       (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value));
DECL_GLPROC(PFNGLVALIDATEPROGRAMPIPELINEPROC,           glValidateProgramPipeline,          void,       (GLuint pipeline));
DECL_GLPROC(PFNGLGETPROGRAMPIPELINEINFOLOGPROC,         glGetProgramPipelineInfoLog,        void,       (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog));
DECL_GLPROC(PFNGLBINDIMAGETEXTUREPROC,                  glBindImageTexture,                 void,       (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format));
DECL_GLPROC(PFNGLGETBOOLEANI_VPROC,                     glGetBooleani_v,                    void,       (GLenum target, GLuint index, GLboolean *data));
DECL_GLPROC(PFNGLMEMORYBARRIERPROC,                     glMemoryBarrier,                    void,       (GLbitfield barriers));
DECL_GLPROC(PFNGLMEMORYBARRIERBYREGIONPROC,             glMemoryBarrierByRegion,            void,       (GLbitfield barriers));
DECL_GLPROC(PFNGLTEXSTORAGE2DMULTISAMPLEPROC,           glTexStorage2DMultisample,          void,       (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations));
DECL_GLPROC(PFNGLGETMULTISAMPLEFVPROC,                  glGetMultisamplefv,                 void,       (GLenum pname, GLuint index, GLfloat* val));
DECL_GLPROC(PFNGLSAMPLEMASKIPROC,                       glSampleMaski,                      void,       (GLuint maskNumber, GLbitfield mask));
DECL_GLPROC(PFNGLGETTEXLEVELPARAMETERIVPROC,            glGetTexLevelParameteriv,           void,       (GLenum target, GLint level, GLenum pname, GLint* params));
DECL_GLPROC(PFNGLGETTEXLEVELPARAMETERFVPROC,            glGetTexLevelParameterfv,           void,       (GLenum target, GLint level, GLenum pname, GLfloat* params));
DECL_GLPROC(PFNGLBINDVERTEXBUFFERPROC,                  glBindVertexBuffer,                 void,       (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride));
DECL_GLPROC(PFNGLVERTEXATTRIBFORMATPROC,                glVertexAttribFormat,               void,       (GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset));
DECL_GLPROC(PFNGLVERTEXATTRIBIFORMATPROC,               glVertexAttribIFormat,              void,       (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset));
DECL_GLPROC(PFNGLVERTEXATTRIBBINDINGPROC,               glVertexAttribBinding,              void,       (GLuint attribindex, GLuint bindingindex));
DECL_GLPROC(PFNGLVERTEXBINDINGDIVISORPROC,              glVertexBindingDivisor,             void,       (GLuint bindingindex, GLuint divisor));

#endif // /GL_ES_VERSION_3_1

#if GL_ES_VERSION_3_2

DECL_GLPROC(PFNGLBLENDBARRIERPROC,                      glBlendBarrier,                     void,       (void));
DECL_GLPROC(PFNGLCOPYIMAGESUBDATAPROC,                  glCopyImageSubData,                 void,       (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth));
DECL_GLPROC(PFNGLDEBUGMESSAGECONTROLPROC,               glDebugMessageControl,              void,       (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled));
DECL_GLPROC(PFNGLDEBUGMESSAGEINSERTPROC,                glDebugMessageInsert,               void,       (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf));
DECL_GLPROC(PFNGLDEBUGMESSAGECALLBACKPROC,              glDebugMessageCallback,             void,       (GLDEBUGPROC callback, const void *userParam));
DECL_GLPROC(PFNGLGETDEBUGMESSAGELOGPROC,                glGetDebugMessageLog,               GLuint,       (GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei* lengths, GLchar* messageLog));
DECL_GLPROC(PFNGLPUSHDEBUGGROUPPROC,                    glPushDebugGroup,                   void,       (GLenum source, GLuint id, GLsizei length, const GLchar *message));
DECL_GLPROC(PFNGLPOPDEBUGGROUPPROC,                     glPopDebugGroup,                    void,       (void));
DECL_GLPROC(PFNGLOBJECTLABELPROC,                       glObjectLabel,                      void,       (GLenum identifier, GLuint name, GLsizei length, const GLchar *label));
DECL_GLPROC(PFNGLGETOBJECTLABELPROC,                    glGetObjectLabel,                   void,       (GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label));
DECL_GLPROC(PFNGLOBJECTPTRLABELPROC,                    glObjectPtrLabel,                   void,       (const void *ptr, GLsizei length, const GLchar *label));
DECL_GLPROC(PFNGLGETOBJECTPTRLABELPROC,                 glGetObjectPtrLabel,                void,       (const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label));
DECL_GLPROC(PFNGLGETPOINTERVPROC,                       glGetPointerv,                      void,       (GLenum pname, void **params));
DECL_GLPROC(PFNGLENABLEIPROC,                           glEnablei,                          void,       (GLenum target, GLuint index));
DECL_GLPROC(PFNGLDISABLEIPROC,                          glDisablei,                         void,       (GLenum target, GLuint index));
DECL_GLPROC(PFNGLBLENDEQUATIONIPROC,                    glBlendEquationi,                   void,       (GLuint buf, GLenum mode));
DECL_GLPROC(PFNGLBLENDEQUATIONSEPARATEIPROC,            glBlendEquationSeparatei,           void,       (GLuint buf, GLenum modeRGB, GLenum modeAlpha));
DECL_GLPROC(PFNGLBLENDFUNCIPROC,                        glBlendFunci,                       void,       (GLuint buf, GLenum src, GLenum dst));
DECL_GLPROC(PFNGLBLENDFUNCSEPARATEIPROC,                glBlendFuncSeparatei,               void,       (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha));
DECL_GLPROC(PFNGLCOLORMASKIPROC,                        glColorMaski,                       void,       (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a));
DECL_GLPROC(PFNGLISENABLEDIPROC,                        glIsEnabledi,                       GLboolean,  (GLenum target, GLuint index));
DECL_GLPROC(PFNGLDRAWELEMENTSBASEVERTEXPROC,            glDrawElementsBaseVertex,           void,       (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex));
DECL_GLPROC(PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC,       glDrawRangeElementsBaseVertex,      void,       (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices, GLint basevertex));
DECL_GLPROC(PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC,   glDrawElementsInstancedBaseVertex,  void,       (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex));
DECL_GLPROC(PFNGLFRAMEBUFFERTEXTUREPROC,                glFramebufferTexture,               void,       (GLenum target, GLenum attachment, GLuint texture, GLint level));
DECL_GLPROC(PFNGLPRIMITIVEBOUNDINGBOXPROC,              glPrimitiveBoundingBox,             void,       (GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW));
DECL_GLPROC(PFNGLGETGRAPHICSRESETSTATUSPROC,            glGetGraphicsResetStatus,           GLenum,     (void));
DECL_GLPROC(PFNGLREADNPIXELSPROC,                       glReadnPixels,                      void,       (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data));
DECL_GLPROC(PFNGLGETNUNIFORMFVPROC,                     glGetnUniformfv,                    void,       (GLuint program, GLint location, GLsizei bufSize, GLfloat *params));
DECL_GLPROC(PFNGLGETNUNIFORMIVPROC,                     glGetnUniformiv,                    void,       (GLuint program, GLint location, GLsizei bufSize, GLint *params));
DECL_GLPROC(PFNGLGETNUNIFORMUIVPROC,                    glGetnUniformuiv,                   void,       (GLuint program, GLint location, GLsizei bufSize, GLuint *params));
DECL_GLPROC(PFNGLMINSAMPLESHADINGPROC,                  glMinSampleShading,                 void,       (GLfloat value));
DECL_GLPROC(PFNGLPATCHPARAMETERIPROC,                   glPatchParameteri,                  void,       (GLenum pname, GLint value));
DECL_GLPROC(PFNGLTEXPARAMETERIIVPROC,                   glTexParameterIiv,                  void,       (GLenum target, GLenum pname, const GLint *params));
DECL_GLPROC(PFNGLTEXPARAMETERIUIVPROC,                  glTexParameterIuiv,                 void,       (GLenum target, GLenum pname, const GLuint *params));
DECL_GLPROC(PFNGLGETTEXPARAMETERIIVPROC,                glGetTexParameterIiv,               void,       (GLenum target, GLenum pname, GLint *params));
DECL_GLPROC(PFNGLGETTEXPARAMETERIUIVPROC,               glGetTexParameterIuiv,              void,       (GLenum target, GLenum pname, GLuint *params));
DECL_GLPROC(PFNGLSAMPLERPARAMETERIIVPROC,               glSamplerParameterIiv,              void,       (GLuint sampler, GLenum pname, const GLint *param));
DECL_GLPROC(PFNGLSAMPLERPARAMETERIUIVPROC,              glSamplerParameterIuiv,             void,       (GLuint sampler, GLenum pname, const GLuint *param));
DECL_GLPROC(PFNGLGETSAMPLERPARAMETERIIVPROC,            glGetSamplerParameterIiv,           void,       (GLuint sampler, GLenum pname, GLint *params));
DECL_GLPROC(PFNGLGETSAMPLERPARAMETERIUIVPROC,           glGetSamplerParameterIuiv,          void,       (GLuint sampler, GLenum pname, GLuint *params));
DECL_GLPROC(PFNGLTEXBUFFERPROC,                         glTexBuffer,                        void,       (GLenum target, GLenum internalformat, GLuint buffer));
DECL_GLPROC(PFNGLTEXBUFFERRANGEPROC,                    glTexBufferRange,                   void,       (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size));
DECL_GLPROC(PFNGLTEXSTORAGE3DMULTISAMPLEPROC,           glTexStorage3DMultisample,          void,       (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations));

#endif // /GL_ES_VERSION_3_2

#endif // /__APPLE__



// ================================================================================
