/*
 * Shader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_SHADER_H__
#define __LLGL_SHADER_H__


#include "Export.h"
#include <string>


namespace LLGL
{


//! Shader base interface.
class LLGL_EXPORT Shader
{

    public:

        virtual ~Shader()
        {
        }

        /**
        \brief Compiles the specified shader source.
        \param[in] shaderSource Specifies the shader source code.
        \return True on success, otherwise "QueryInfoLog" can be used to query the reason for failure.
        \see QueryInfoLog
        */
        virtual bool Compile(const std::string& shaderSource) = 0;

        //! Returns the information log after the shader compilation.
        virtual std::string QueryInfoLog() = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
