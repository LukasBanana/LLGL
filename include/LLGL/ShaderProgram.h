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
#include "VertexAttribute.h"
#include <string>
#include <vector>


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
        \return True on success, otherwise "QueryInfoLog" can be used to query the reason for failure.
        \remarks Each attached shader must be compiled first!
        \see QueryInfoLog
        */
        virtual bool LinkShaders() = 0;

        //! Returns the information log after the shader linkage.
        virtual std::string QueryInfoLog() = 0;

        /**
        \brief Binds the specified vertex attributes to this shader program.
        \remarks This is only required for a shader program, which has an attached vertex shader,
        and it can only be used after the shaders have already been linked.
        \see AttachShader(VertexShader&)
        \see LinkShaders
        \throws std::invalid_argument If the name of an vertex attribute is invalid or the maximal number of available vertex attributes is exceeded.
        \throws std::runtime_error If this function is called before the shaders have been successfully linked.
        */
        virtual void BindVertexAttributes(const std::vector<VertexAttribute>& vertexAttribs) = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
