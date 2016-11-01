/*
 * GLSampler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_SAMPLER_H
#define LLGL_GL_SAMPLER_H


#include "../OpenGL.h"
#include <LLGL/Sampler.h>


namespace LLGL
{


class GLSampler : public Sampler
{

    public:

        GLSampler();
        ~GLSampler();

        void SetDesc(const SamplerDescriptor& desc);

        //! Returns the hardware sampler ID.
        inline GLuint GetID() const
        {
            return id_;
        }

    private:

        GLuint id_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
