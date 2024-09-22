/*
 * GLEmulatedSampler.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_EMULATED_SAMPLER_H
#define LLGL_GL_EMULATED_SAMPLER_H


#include <LLGL/Sampler.h>
#include "../OpenGL.h"
#include <memory>


namespace LLGL
{


// This class emulates the Sampler-Object (GL_ARB_sampler_objects) functionality, which is only available since GL 3.3.
class GLEmulatedSampler final : public Sampler
{

    public:

        // Converts and stores the sampler descriptor to GL states.
        void SamplerParameters(const SamplerDescriptor& desc);

        // Binds all attributes of this sampler to the specified GL texture object.
        void BindTexParameters(GLenum target, const GLEmulatedSampler* prevSampler = nullptr) const;

    public:

        // Compares the two GLEmulatedSampler objects in a strict-weak-order (SWO).
        static int CompareSWO(const GLEmulatedSampler& lhs, const GLEmulatedSampler& rhs);

    private:

        GLint   wrapS_              = GL_REPEAT;
        GLint   wrapT_              = GL_REPEAT;
        GLint   wrapR_              = GL_REPEAT;
        GLint   minFilter_          = GL_NEAREST_MIPMAP_LINEAR;
        GLint   magFilter_          = GL_LINEAR;
        #if LLGL_OPENGL
        GLfloat maxAnisotropy_      = 0.0f;
        #endif
        GLfloat minLod_             = -1000.0f;
        GLfloat maxLod_             = +1000.0f;
        #if LLGL_OPENGL
        GLfloat lodBias_            = 0.0f;
        #endif
        GLint   compareMode_        = GL_NONE;
        GLint   compareFunc_        = GL_LESS;
        #if LLGL_SAMPLER_BORDER_COLOR
        GLfloat borderColor_[4]     = { 0.0f, 0.0f, 0.0f, 0.0f };
        bool    borderColorUsed_    = false;
        #endif

};

using GLEmulatedSamplerPtr = std::unique_ptr<GLEmulatedSampler>;


} // /namespace LLGL


#endif



// ================================================================================
