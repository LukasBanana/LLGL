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

/**
\brief Shader stage flags.
\remarks Specifies which shader stages are affected by a state change,
e.g. at which shader stages a constant buffer is set.
For the render systems, which do not support these flags, always all shader stages are affected.
\note Only supported with: Direct3D 11
*/
struct ShaderStageFlags
{
    enum
    {
        VertexStage         = (1 << 0), //!< Specifies the vertex shader stage.
        TessControlStage    = (1 << 1), //!< Specifies the tessellation-control shader stage (also "Hull Shader").
        TessEvaluationStage = (1 << 2), //!< Specifies the tessellation-evaluation shader stage (also "Domain Shader").
        GeometryStage       = (1 << 3), //!< Specifies the geometry shader stage.
        FragmentStage       = (1 << 4), //!< Specifies the fragment shader stage (also "Pixel Shader").
        ComputeStage        = (1 << 5), //!< Specifies the compute shader stage.

        //! Specifies all tessellation stages, i.e. tessellation-control-, tessellation-evaluation shader stages.
        AllTessStages       = (TessControlStage | TessEvaluationStage),

        //! Specifies all graphics pipeline shader stages, i.e. vertex-, tessellation-, geometry-, and fragment shader stages.
        AllGraphicsStages   = (VertexStage | AllTessStages | GeometryStage | FragmentStage),

        //! Specifies all shader stages.
        AllStages           = (AllGraphicsStages | ComputeStage),
    };
};

//! Shader source code union.
union ShaderSource
{
    /**
    \brief Specifies the shader source code GLSL.
    \param[in] sourceCode Specifies the shader source code.
    \note Only supported with: OpenGL (for GLSL).
    */
    ShaderSource(const std::string& sourceCode) :
        sourceHLSL{ sourceCode, "", "", 0 }
    {
    }

    /**
    \brief Specifies the shader source code for HLSL.
    \param[in] sourceCode Specifies the shader source code.
    \param[in] entryPoint Specifies the shader entry point.
    \param[in] target Specifies the shader version target (see https://msdn.microsoft.com/en-us/library/windows/desktop/jj215820(v=vs.85).aspx).
    \param[in] flags Specifies optional compilation flags. This can be a bitwise OR combination of the 'ShaderCompileFlags' enumeration entries. By default 0.
    \note Only supported with: Direct3D 11, Direct3D 12 (for HLSL).
    */
    ShaderSource(const std::string& sourceCode, const std::string& entryPoint, const std::string& target, int flags = 0) :
        sourceHLSL{ sourceCode, entryPoint, target, flags }
    {
    }

    ~ShaderSource()
    {
    }

    //! Shader source descriptor for GLSL.
    struct GLSL
    {
        const std::string& sourceCode; //!< Shader source code string.
    }
    sourceGLSL;

    //! Shader source descriptor for HLSL.
    struct HLSL
    {
        const std::string&  sourceCode; //!< Shader source code string.
        std::string         entryPoint; //!< Shader entry point (this is the name of the shader main function).
        std::string         target;     //!< Shader version target (see https://msdn.microsoft.com/en-us/library/windows/desktop/jj215820(v=vs.85).aspx).
        int                 flags;      //!< Optional compilation flags. This can be a bitwise OR combination of the 'ShaderCompileFlags' enumeration entries.
    }
    sourceHLSL;
};


//! Shader interface.
class LLGL_EXPORT Shader
{

    public:

        virtual ~Shader();

        /**
        \brief Compiles the specified shader source.
        \param[in] shaderSource Specifies the shader source code.
        \return True on success, otherwise "QueryInfoLog" can be used to query the reason for failure.
        \see QueryInfoLog
        */
        virtual bool Compile(const ShaderSource& shaderSource) = 0;

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
