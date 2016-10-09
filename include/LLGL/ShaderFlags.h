/*
 * ShaderFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_SHADER_FLAGS_H__
#define __LLGL_SHADER_FLAGS_H__


#include "Export.h"
#include "StreamOutputFormat.h"
#include <string>


namespace LLGL
{


/* ----- Enumerations ----- */

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


/* ----- Flags ----- */

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
e.g. to which shader stages a constant buffer is set.
For the render systems, which do not support these flags, always all shader stages are affected.
\note Only supported with: Direct3D 11, Direct3D 12
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

        /**
        \brief Specifies whether a resource is bound to the shader stages for reading only.
        \remarks This can be used to set the shader-resource-view (SRV) of a storage buffer to the shader stages
        instead of the unordered-access-view (UAV), which is the default, if the storage buffer has such a UAV.
        */
        ReadOnlyResource    = (1 << 6),

        //! Specifies all tessellation stages, i.e. tessellation-control-, tessellation-evaluation shader stages.
        AllTessStages       = (TessControlStage | TessEvaluationStage),

        //! Specifies all graphics pipeline shader stages, i.e. vertex-, tessellation-, geometry-, and fragment shader stages.
        AllGraphicsStages   = (VertexStage | AllTessStages | GeometryStage | FragmentStage),

        //! Specifies all shader stages.
        AllStages           = (AllGraphicsStages | ComputeStage),
    };
};


/* ----- Structures ----- */

//! Shader source code structure.
struct ShaderSource
{
    /**
    \brief Constructor with shader source code for GLSL.
    \param[in] sourceCode Specifies the shader source code.
    \note Only supported with: OpenGL.
    */
    inline ShaderSource(const std::string& sourceCode) :
        sourceCode( sourceCode )
    {
    }

    /**
    \brief Constructor with shader source code for GLSL.
    \param[in] sourceCode Specifies the shader source code with move semantic.
    \note Only supported with: OpenGL.
    */
    inline ShaderSource(std::string&& sourceCode) :
        sourceCode( std::move(sourceCode) )
    {
    }

    /**
    \brief Constructor with shader source code for HLSL.
    \param[in] sourceCode Specifies the shader source code.
    \param[in] entryPoint Specifies the shader entry point.
    \param[in] target Specifies the shader version target (see https://msdn.microsoft.com/en-us/library/windows/desktop/jj215820(v=vs.85).aspx).
    \param[in] flags Specifies optional compilation flags. This can be a bitwise OR combination of the 'ShaderCompileFlags' enumeration entries. By default 0.
    \see ShaderCompileFlags
    \note Only supported with: Direct3D 11, Direct3D 12.
    */
    inline ShaderSource(const std::string& sourceCode, const std::string& entryPoint, const std::string& target, long flags = 0) :
        sourceCode  { sourceCode                },
        sourceHLSL  { entryPoint, target, flags }
    {
    }

    /**
    \brief Constructor with shader source code for HLSL.
    \param[in] sourceCode Specifies the shader source code with move semantic.
    \param[in] entryPoint Specifies the shader entry point.
    \param[in] target Specifies the shader version target (see https://msdn.microsoft.com/en-us/library/windows/desktop/jj215820(v=vs.85).aspx).
    \param[in] flags Specifies optional compilation flags. This can be a bitwise OR combination of the 'ShaderCompileFlags' enumeration entries. By default 0.
    \see ShaderCompileFlags
    \note Only supported with: Direct3D 11, Direct3D 12.
    */
    inline ShaderSource(std::string&& sourceCode, const std::string& entryPoint, const std::string& target, long flags = 0) :
        sourceCode  { std::move(sourceCode)     },
        sourceHLSL  { entryPoint, target, flags }
    {
    }

    //! Additional descripor for HLSL shader source.
    struct SourceHLSL
    {
        std::string         entryPoint; //!< Shader entry point (this is the name of the shader main function).
        std::string         target;     //!< Shader version target (see https://msdn.microsoft.com/en-us/library/windows/desktop/jj215820(v=vs.85).aspx).
        long                flags;      //!< Optional compilation flags. This can be a bitwise OR combination of the 'ShaderCompileFlags' enumeration entries.
    };

    //! Additional descriptor for stream outputs.
    struct StreamOutput
    {
        StreamOutputFormat  format;     //!< Stream-output buffer format.
    };

    std::string     sourceCode;     //!< Shader source code string.
    SourceHLSL      sourceHLSL;     //!< Additional HLSL shader source descriptor.
    StreamOutput    streamOutput;   //!< Optional stream output for a geometry shader (or a vertex shader when used with OpenGL).
};


} // /namespace LLGL


#endif



// ================================================================================
