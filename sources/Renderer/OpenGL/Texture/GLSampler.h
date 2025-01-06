/*
 * GLSampler.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_SAMPLER_H
#define LLGL_GL_SAMPLER_H


#include <LLGL/Sampler.h>
#include "../OpenGL.h"
#include <memory>


namespace LLGL
{


class GLSampler final : public Sampler
{

    public:

        #include <LLGL/Backend/Sampler.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        GLSampler(const char* debugName = nullptr);
        ~GLSampler();

        // Sets the GL sampler parameters with the specified descriptor, i.e. glSamplerParameter*.
        void SamplerParameters(const SamplerDescriptor& desc);

        // Returns the hardware sampler ID.
        inline GLuint GetID() const
        {
            return id_;
        }

    private:

        GLuint id_ = 0;

};

using GLSamplerSPtr = std::shared_ptr<GLSampler>;


} // /namespace LLGL


#endif



// ================================================================================
