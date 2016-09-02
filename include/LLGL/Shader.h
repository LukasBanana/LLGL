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
    TessControl,    //!< Tessellation control shader type (also "Hull Shader").
    TessEvaluation, //!< Tessellation evaluation shader type (also "Domain Shader").
    Geometry,       //!< Geometry shader type.
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
        \remarks This function can only be used for GLSL, because HLSL needs more information such as an entry point and target.
        \see QueryInfoLog
        \see Compile(const std::string&, const std::string&, const std::string&)
        */
        virtual bool Compile(const std::string& shaderSource) = 0;

        /**
        \brief Compiles the specified shader source with the specified parameters.
        \param[in] shaderSource Specifies the shader source code.
        \param[in] entryPoint Specifies the shader entry point.
        \param[in] target Specifies the shader version target (see https://msdn.microsoft.com/en-us/library/windows/desktop/jj215820(v=vs.85).aspx).
        \return True on success, otherwise "QueryInfoLog" can be used to query the reason for failure.
        \remarks This function should be used for HLSL.
        */
        virtual bool Compile(const std::string& shaderSource, const std::string& entryPoint, const std::string& target) = 0;

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
