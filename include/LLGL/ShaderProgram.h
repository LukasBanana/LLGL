/*
 * ShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_SHADER_PROGRAM_H__
#define __LLGL_SHADER_PROGRAM_H__


#include "Export.h"
#include "VertexShader.h"
#include "FragmentShader.h"
#include "GeometryShader.h"
#include "TessControlShader.h"
#include "TessEvaluationShader.h"
#include "ComputeShader.h"
#include <string>


namespace LLGL
{


//! Shader program interface.
class LLGL_EXPORT ShaderProgram
{

    public:

        virtual ~ShaderProgram()
        {
        }

        virtual void AttachShader( VertexShader&         vertexShader         ) = 0;
        virtual void AttachShader( FragmentShader&       fragmentShader       ) = 0;
        virtual void AttachShader( GeometryShader&       geometryShader       ) = 0;
        virtual void AttachShader( TessControlShader&    tessControlShader    ) = 0;
        virtual void AttachShader( TessEvaluationShader& tessEvaluationShader ) = 0;
        virtual void AttachShader( ComputeShader&        computeShader        ) = 0;

        /**
        \brief Links all attached shaders to the final shader program.
        \remarks Each attached shader must be compiled first!
        */
        virtual bool LinkShaders() = 0;

        virtual std::string QueryInfoLog() = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
