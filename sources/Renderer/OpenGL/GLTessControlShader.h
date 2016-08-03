/*
 * GLTessControlShader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_TESS_CONTROL_SHADER_H__
#define __LLGL_GL_TESS_CONTROL_SHADER_H__


#include <LLGL/TessControlShader.h>
#include "GLHardwareShader.h"


namespace LLGL
{


class GLTessControlShader : public TessControlShader
{

    public:

        GLTessControlShader();

        bool Compile(const std::string& shaderSource) override;

        std::string QueryInfoLog() override;

        GLHardwareShader hwShader;

};


} // /namespace LLGL


#endif



// ================================================================================
