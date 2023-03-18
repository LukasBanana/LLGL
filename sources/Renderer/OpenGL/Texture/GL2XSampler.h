/*
 * GL2XSampler.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL2X_SAMPLER_H
#define LLGL_GL2X_SAMPLER_H


#include <LLGL/Sampler.h>
#include "../OpenGL.h"
#include <memory>


namespace LLGL
{


// This class emulates the Sampler-Object (GL_ARB_sampler_objects) functionality, for GL 2.x.
class GL2XSampler final : public Sampler
{

    public:

        // Converts and stores the sampler descriptor to GL states.
        void SamplerParameters(const SamplerDescriptor& desc);

        // Binds all attributes of this sampler to the specified GL texture object.
        void BindTexParameters(GLenum target, const GL2XSampler* prevSampler = nullptr) const;

    public:

        // Compares the two GL2XSampler objects in a strict-weak-order (SWO).
        static int CompareSWO(const GL2XSampler& lhs, const GL2XSampler& rhs);

    private:

        GLint   wrapS_              = GL_REPEAT;
        GLint   wrapT_              = GL_REPEAT;
        GLint   wrapR_              = GL_REPEAT;
        GLint   minFilter_          = GL_NEAREST_MIPMAP_LINEAR;
        GLint   magFilter_          = GL_LINEAR;
        GLfloat maxAnisotropy_      = 0.0f;
        GLfloat minLod_             = -1000.0f;
        GLfloat maxLod_             = +1000.0f;
        GLfloat lodBias_            = 0.0f;
        GLint   compareMode_        = GL_NONE;
        GLint   compareFunc_        = GL_LESS;
        GLfloat borderColor_[4]     = { 0.0f, 0.0f, 0.0f, 0.0f };
        bool    borderColorUsed_    = false;

};

using GL2XSamplerPtr = std::unique_ptr<GL2XSampler>;


} // /namespace LLGL


#endif



// ================================================================================
