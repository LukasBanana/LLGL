/*
 * GLSampler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_SAMPLER_H
#define LLGL_GL_SAMPLER_H


#include "../OpenGL.h"
#include <LLGL/Sampler.h>


namespace LLGL
{


class GLSampler final : public Sampler
{

    public:

        void SetName(const char* name) override;

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
