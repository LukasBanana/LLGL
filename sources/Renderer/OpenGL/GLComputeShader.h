/*
 * GLComputeShader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_COMPUTE_SHADER_H__
#define __LLGL_GL_COMPUTE_SHADER_H__


#include <LLGL/ComputeShader.h>
#include "GLHardwareShader.h"


namespace LLGL
{


class GLComputeShader : public ComputeShader
{

    public:

        GLComputeShader() :
            hwShader(GL_COMPUTE_SHADER)
        {
        }

        GLHardwareShader hwShader;

};


} // /namespace LLGL


#endif



// ================================================================================
