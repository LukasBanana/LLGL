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


//! Shader type enumeration.
enum class ShaderType
{
    Vertex,         //!< Vertex shader type.
    Geometry,       //!< Geometry shader type.
    TessControl,    //!< Tessellation control shader type (also "Hull Shader").
    TessEvaluation, //!< Tessellation evaluation shader type (also "Domain Shader").
    Fragment,       //!< Fragment shader type (also "Pixel Shader").
    Compute,        //!< Compute shader type.
};


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

        //! Returns the type of this shader.
        inline ShaderType GetType() const
        {
            return type_;
        }

    protected:

        Shader(const ShaderType type) :
            type_( type )
        {
        }

    private:

        ShaderType type_;

};


} // /namespace LLGL


#endif



// ================================================================================
