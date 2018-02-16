/*
 * GLSamplerArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_SAMPLER_ARRAY_H
#define LLGL_GL_SAMPLER_ARRAY_H


#include <LLGL/SamplerArray.h>
#include "../OpenGL.h"
#include <vector>
#include <cstdint>


namespace LLGL
{


class Sampler;

class GLSamplerArray : public SamplerArray
{

    public:

        GLSamplerArray(std::uint32_t numSamplers, Sampler* const * samplerArray);

        //! Returns the array of sampler IDs.
        inline const std::vector<GLuint>& GetIDArray() const
        {
            return idArray_;
        }

    private:

        std::vector<GLuint> idArray_;

};


} // /namespace LLGL


#endif



// ================================================================================
