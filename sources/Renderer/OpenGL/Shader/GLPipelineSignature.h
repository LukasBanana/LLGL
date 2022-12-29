/*
 * GLPipelineSignature.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_PIPELINE_SIGNATURE_H
#define LLGL_GL_PIPELINE_SIGNATURE_H


#include "../OpenGL.h"
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
        GLPipelineSignature(std::size_t numShaders, const Shader* const* shaders);

        /*
        Initializes the signature with the specified shaders.
        The internal ID array is sorted by their shader types for matching SWO comparison.
        The number of shaders must be less than or equal to LLGL_MAX_NUM_GL_SHADERS_PER_PIPELINE.
        */
        void Build(std::size_t numShaders, const Shader* const* shaders);

        // Returns a signed integer of the strict-weak-order (SWO) comparison, and 0 on equality.
        static int CompareSWO(const GLPipelineSignature& lhs, const GLPipelineSignature& rhs);

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
        GLuint numShaders_                                      = 0;
        GLuint shaders_[LLGL_MAX_NUM_GL_SHADERS_PER_PIPELINE]   = {};

};


} // /namespace LLGL


#endif



// ================================================================================
