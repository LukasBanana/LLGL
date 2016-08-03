/*
 * GLVertexShader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_VERTEX_SHADER_H__
#define __LLGL_GL_VERTEX_SHADER_H__


#include <LLGL/VertexShader.h>
#include "GLHardwareShader.h"


namespace LLGL
{


class GLVertexShader : public VertexShader
{

    public:

        GLVertexShader() :
            hwShader(GL_VERTEX_SHADER)
        {
        }

        GLHardwareShader hwShader;

};


} // /namespace LLGL


#endif



// ================================================================================
