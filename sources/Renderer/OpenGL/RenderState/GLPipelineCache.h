/*
 * GLPipelineCache.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_PIPELINE_CACHE_H
#define LLGL_GL_PIPELINE_CACHE_H


#include "../OpenGL.h"
#include "../Shader/GLShader.h"
#include <LLGL/PipelineCache.h>
#include <LLGL/Container/DynamicArray.h>


namespace LLGL
{


class GLPipelineCache final : public PipelineCache
{

    public:

        GLPipelineCache() = default;

        GLPipelineCache(const Blob& initialBlob);

        Blob GetBlob() const override;

    public:

        // Returns true if this pipeline cache has a GL program binary blob.
        inline bool HasProgramBinary(GLShader::Permutation permutation) const
        {
            return !(entries_[permutation].data.empty());
        }

        // Loads the pipeline cache into the specified GL shader program.
        bool ProgramBinary(GLShader::Permutation permutation, GLuint program);

        // Retrieves the pipeline cache from the specified GL shader program.
        bool GetProgramBinary(GLShader::Permutation permutation, GLuint program);

    private:

        struct CacheEntry
        {
            GLenum              format  = 0;
            GLsizei             length  = 0;
            DynamicByteArray    data;
        };

    private:

        void InitializeEntry(GLShader::Permutation permutation, const void* data);

    private:

        CacheEntry entries_[GLShader::PermutationCount];

};


} // /namespace LLGL


#endif



// ================================================================================
