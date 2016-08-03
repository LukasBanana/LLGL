/*
 * GLGeometryShader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_GEOMETRY_SHADER_H__
#define __LLGL_GL_GEOMETRY_SHADER_H__


#include <LLGL/GeometryShader.h>
#include "GLHardwareShader.h"


namespace LLGL
{


class GLGeometryShader : public GeometryShader
{

    public:

        GLGeometryShader() :
            hwShader(GL_GEOMETRY_SHADER)
        {
        }

        GLHardwareShader hwShader;

};


} // /namespace LLGL


#endif



// ================================================================================
