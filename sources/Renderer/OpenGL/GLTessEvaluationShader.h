/*
 * GLTessEvaluationShader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_TESS_EVALUATION_SHADER_H__
#define __LLGL_GL_TESS_EVALUATION_SHADER_H__


#include <LLGL/TessEvaluationShader.h>
#include "GLHardwareShader.h"


namespace LLGL
{


class GLTessEvaluationShader : public TessEvaluationShader
{

    public:

        GLTessEvaluationShader();

        bool Compile(const std::string& shaderSource) override;

        std::string QueryInfoLog() override;

        GLHardwareShader hwShader;

};


} // /namespace LLGL


#endif



// ================================================================================
