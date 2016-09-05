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

//! Shader compilation flags enumeration.
struct ShaderCompileFlags
{
    enum
    {
        Debug       = (1 << 0), //!< Insert debug information.
        O1          = (1 << 1), //!< Optimization level 1.
        O2          = (1 << 2), //!< Optimization level 2.
        O3          = (1 << 3), //!< Optimization level 3.
        WarnError   = (1 << 4), //!< Warnings are treated as errors.
    };
};

//! Shader disassemble flags enumeration.
struct ShaderDisassembleFlags
{
    enum
    {
        InstructionOnly = (1 << 0), //!< Show only instructions in disassembly output.
    };
};


//! Shader base interface.
class LLGL_EXPORT Shader
{

    public:

        virtual ~Shader();

        /**
        \brief Compiles the specified shader source.
        \param[in] shaderSource Specifies the shader source code.
        \return True on success, otherwise "QueryInfoLog" can be used to query the reason for failure.
        \note Only supported with: OpenGL (for GLSL).
        \see QueryInfoLog
        \see Compile(const std::string&, const std::string&, const std::string&)
        */
        virtual bool Compile(const std::string& shaderSource) = 0;

        /**
        \brief Compiles the specified shader source with the specified parameters.
        \param[in] shaderSource Specifies the shader source code.
        \param[in] entryPoint Specifies the shader entry point.
        \param[in] target Specifies the shader version target (see https://msdn.microsoft.com/en-us/library/windows/desktop/jj215820(v=vs.85).aspx).
        \param[in] flags Specifies optional compilation flags. This can be a bitwise OR combination of the 'ShaderCompileFlags' enumeration entries. By default 0.
        \return True on success, otherwise "QueryInfoLog" can be used to query the reason for failure.
        \note Only supported with: Direct3D 11, Direct3D 12 (for HLSL).
        \see ShaderCompileFlags
        */
        virtual bool Compile(const std::string& shaderSource, const std::string& entryPoint, const std::string& target, int flags = 0) = 0;

        /**
        \brief Disassembles the previously compiled shader byte code.
        \param[in] flags Specifies optional disassemble flags. This can be a bitwise OR combination of the 'ShaderDisassembleFlags' enumeration entries. By default 0.
        \return Disassembled assembler code or an empty string if disassembling was not possible.
        \note Only supported with: Direct3D 11, Direct3D 12 (for HLSL).
        */
        virtual std::string Disassemble(int flags = 0);

        //! Returns the information log after the shader compilation.
        virtual std::string QueryInfoLog() = 0;

        //! Returns the type of this shader.
        inline ShaderType GetType() const
        {
            return type_;
        }

    protected:

        Shader(const ShaderType type);

    private:

        ShaderType type_;

};


} // /namespace LLGL


#endif



// ================================================================================
