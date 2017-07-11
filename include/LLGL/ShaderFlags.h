/*
 * ShaderFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SHADER_FLAGS_H
#define LLGL_SHADER_FLAGS_H


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

//! Shader source and binary code descriptor structure.
struct ShaderDescriptor
{
    ShaderDescriptor() = default;

    ShaderDescriptor(const std::string& entryPoint, const std::string& target, long flags = 0) :
        entryPoint { entryPoint },
        target     { target     }
    {
    }

    //! Additional descriptor for stream outputs.
    struct StreamOutput
    {
        StreamOutputFormat format;  //!< Stream-output buffer format.
    };

    /**
    \brief Shader entry point (shader main function).
    \note Only supported with: Direct3D 11, Direct3D 12.
    */
    std::string     entryPoint;

    /*
    \brief Shader target profile (e.g. "vs_5_0" for vertex shader model 5.0).
    \note Only supported with: Direct3D 11, Direct3D 12.
    \see https://msdn.microsoft.com/en-us/library/windows/desktop/jj215820(v=vs.85).aspx
    */
    std::string     target;

    /**
    \brief Optional compilation flags. By default 0.
    \remarks This can be a bitwise OR combination of the 'ShaderCompileFlags' enumeration entries.
    \note Only supported with: Direct3D 11, Direct3D 12.
    \see ShaderCompileFlags
    */
    long            flags           = 0;

    //! Optional stream output descriptor for a geometry shader (or a vertex shader when used with OpenGL).
    StreamOutput    streamOutput;
};


} // /namespace LLGL


#endif



// ================================================================================
