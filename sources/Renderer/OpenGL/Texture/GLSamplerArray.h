/*
 * GLSamplerArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_SAMPLER_ARRAY_H__
#define __LLGL_GL_SAMPLER_ARRAY_H__


#include <LLGL/SamplerArray.h>
#include "../OpenGL.h"
#include <vector>


namespace LLGL
{


class Sampler;

class GLSamplerArray : public SamplerArray
{

    public:

        GLSamplerArray(unsigned int numSamplers, Sampler* const * samplerArray);

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
