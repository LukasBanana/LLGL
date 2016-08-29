/*
 * GLSamplerState.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_SAMPLER_STATE_H__
#define __LLGL_GL_SAMPLER_STATE_H__


#include "../OpenGL.h"
#include <LLGL/SamplerState.h>


namespace LLGL
{


class GLSamplerState : public SamplerState
{

    public:

        GLSamplerState();
        ~GLSamplerState();

        void SetDesc(const SamplerStateDescriptor& desc);

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
