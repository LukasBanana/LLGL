/*
 * GLPipelineSignature.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_PIPELINE_SIGNATURE_H
#define LLGL_GL_PIPELINE_SIGNATURE_H


#include "../OpenGL.h"
#include "GLShader.h"
#include <cstddef>


namespace LLGL
{


#define LLGL_MAX_NUM_GL_SHADERS_PER_PIPELINE (5u)

class Shader;

// Helper class to store shader or shader program IDs for SWO comparison in GLStatePool.
class GLPipelineSignature
{

    public:

        GLPipelineSignature() = default;

        // Initializes the signature with the specified shaders. Equivalent of calling Build.
        GLPipelineSignature(std::size_t numShaders, const Shader* const* shaders, GLShader::Permutation permutation);

        /*
        Initializes the signature with the specified shaders.
        The internal ID array is sorted by their shader types for matching SWO comparison.
        The number of shaders must be less than or equal to LLGL_MAX_NUM_GL_SHADERS_PER_PIPELINE.
        */
        void Build(std::size_t numShaders, const Shader* const* shaders, GLShader::Permutation permutation);

    public:

        // Returns a signed integer of the strict-weak-order (SWO) comparison, and 0 on equality.
        static int CompareSWO(const GLPipelineSignature& lhs, const GLPipelineSignature& rhs);

        // Returns the last shader in the pipeline that modifies gl_Position.
        static const GLShader* FindFinalGLPositionShader(std::size_t numShaders, const Shader* const* shaders);

    public:

        // Returns the number of shaders in this pipeline.
        inline GLuint GetNumShaders() const
        {
            return numShaders_;
        }

        // Returns a pointer to the array of shader IDs.
        inline const GLuint* GetShaders() const
        {
            return shaders_;
        }

    private:

        // IDs of shaders in this pipeline: glCreateShader/glCreateProgram/glCreateShaderProgramv
        union
        {
            struct
            {
                GLuint isSeparablePpeline_  :  1;
                GLuint numShaders_          : 31;
            };
            GLuint typeBitAndNumShaders_;
        };
        GLuint shaders_[LLGL_MAX_NUM_GL_SHADERS_PER_PIPELINE] = {};

};


} // /namespace LLGL


#endif



// ================================================================================
