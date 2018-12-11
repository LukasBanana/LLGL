/*
 * ShaderFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SHADER_FLAGS_H
#define LLGL_SHADER_FLAGS_H


#include "Export.h"
#include "StreamOutputFormat.h"
#include <cstddef>


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Shader type enumeration.
\see ShaderDescriptor::type
*/
enum class ShaderType
{
    Undefined,      //!< Undefined shader type.
    Vertex,         //!< Vertex shader type.
    TessControl,    //!< Tessellation control shader type (also "Hull Shader").
    TessEvaluation, //!< Tessellation evaluation shader type (also "Domain Shader").
    Geometry,       //!< Geometry shader type.
    Fragment,       //!< Fragment shader type (also "Pixel Shader").
    Compute,        //!< Compute shader type.
};

/**
\brief Shader source type enumeration.
\see ShaderDescriptor::sourceType
\see ShaderDescriptor::sourceSize
*/
enum class ShaderSourceType
{
    CodeString,     //!< Refers to <code>sourceSize+1</code> bytes, describing shader high-level code (including null terminator).
    CodeFile,       //!< Refers to <code>sourceSize+1</code> bytes, describing the filename of the shader high-level code (including null terminator).
    BinaryBuffer,   //!< Refers to <code>sourceSize</code> bytes, describing shader binary code.
    BinaryFile,     //!< Refers to <code>sourceSize+1</code> bytes, describing the filename of the shader binary code (including null terminator).
};


/* ----- Flags ----- */

/**
\brief Shader compilation flags enumeration.
\note Only supported with: Direct3D 11, Direct3D 12.
*/
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
\brief Shader stage flags enumeration.
\remarks Specifies which shader stages are affected by a state change, e.g. to which shader stages a constant buffer is bound.
\see BindingDescriptor::stageFlags
*/
struct StageFlags
{
    enum
    {
        //! Specifies the vertex shader stage.
        VertexStage         = (1 << 0),

        //! Specifies the tessellation-control shader stage (also referred to as "Hull Shader").
        TessControlStage    = (1 << 1),

        //! Specifies the tessellation-evaluation shader stage (also referred to as "Domain Shader").
        TessEvaluationStage = (1 << 2),

        //! Specifies the geometry shader stage.
        GeometryStage       = (1 << 3),

        //! Specifies the fragment shader stage (also "Pixel Shader").
        FragmentStage       = (1 << 4),

        //! Specifies the compute shader stage.
        ComputeStage        = (1 << 5),

        //! Specifies all tessellation stages, i.e. tessellation-control-, tessellation-evaluation shader stages.
        AllTessStages       = (TessControlStage | TessEvaluationStage),

        //! Specifies all graphics pipeline shader stages, i.e. vertex-, tessellation-, geometry-, and fragment shader stages.
        AllGraphicsStages   = (VertexStage | AllTessStages | GeometryStage | FragmentStage),

        //! Specifies all shader stages.
        AllStages           = (AllGraphicsStages | ComputeStage),
    };
};


/* ----- Structures ----- */

/**
\brief Shader source and binary code descriptor structure.
\see RenderSystem::CreateShader
*/
struct ShaderDescriptor
{
    ShaderDescriptor() = default;
    ShaderDescriptor(const ShaderDescriptor&) = default;
    ShaderDescriptor& operator = (const ShaderDescriptor&) = default;

    //! Constructor to initialize the shader descriptor with a source filename.
    inline ShaderDescriptor(const ShaderType type, const char* source) :
        type   { type   },
        source { source }
    {
    }

    //! Constructor to initialize the shader descriptor with a source filename, entry point, profile, and optional flags.
    inline ShaderDescriptor(
        const ShaderType    type,
        const char*         source,
        const char*         entryPoint,
        const char*         profile,
        long                flags = 0) :
            type       { type       },
            source     { source     },
            entryPoint { entryPoint },
            profile    { profile    },
            flags      { flags      }
    {
    }

    //! Additional descriptor for stream outputs.
    struct StreamOutput
    {
        StreamOutputFormat format;  //!< Stream-output buffer format.
    };

    //! Specifies the type of the shader, i.e. if it is either a vertex or fragment shader or the like. By default ShaderType::Undefined.
    ShaderType          type        = ShaderType::Undefined;

    /**
    \brief Pointer to the shader source. This is either a null terminated string or a raw byte buffer (depending on the 'sourceType' member).
    \remarks This must not be null when passed to the RenderSystem::CreateShader function.
    If this is raw byte buffer rather than a null terminated string, the 'sourceSize' member must not be zero!
    \see sourceSize
    \see sourceType
    */
    const char*         source      = nullptr;

    /**
    \brief Specifies the size of the shader source (excluding the null terminator).
    \remarks If this is zero, the 'source' member is expected to point to a null terminated string and the size is automatically determined.
    For the binrary buffer source type (i.e. ShaderSourceType::BinaryBuffer), this must not be zero!
    \see source
    */
    std::size_t         sourceSize  = 0;

    /**
    \brief Specifies the type of the shader source. By default ShaderSourceType::CodeFile.
    \remarks With the filename source types (i.e. ShaderSourceType::CodeFile and ShaderSourceType::BinaryFile),
    the shader source or binary code will be loaded from file using the standard C++ file streams (i.e. std::ifstream).
    Only the binary buffer source type (i.e. ShaderSourceType::BinaryBuffer) does not require a null terminator for the 'source' pointer.
    \see ShaderSourceType
    \see source
    */
    ShaderSourceType    sourceType      = ShaderSourceType::CodeFile;

    /**
    \brief Shader entry point (shader main function). If this is null, the empty string is used. By default null.
    \note Only supported with: HLSL, SPIR-V.
    */
    const char*         entryPoint      = nullptr;

    /*
    \brief Shader target profile (e.g. "vs_5_0" for vertex shader model 5.0). If this is null, the empty string is used. By default null.
    \note Only supported with: Direct3D 11, Direct3D 12.
    \see https://msdn.microsoft.com/en-us/library/windows/desktop/jj215820(v=vs.85).aspx
    */
    const char*         profile         = nullptr;

    /**
    \brief Optional compilation flags. By default 0.
    \remarks This can be a bitwise OR combination of the 'ShaderCompileFlags' enumeration entries.
    \note Only supported with: Direct3D 11, Direct3D 12.
    \see ShaderCompileFlags
    */
    long                flags           = 0;

    //! Optional stream output descriptor for a geometry shader (or a vertex shader when used with OpenGL).
    StreamOutput        streamOutput;
};


/* ----- Functions ----- */

/**
\brief Returns true if the specified shader source type is either ShaderSourceType::CodeString or ShaderSourceType::CodeFile.
\see ShaderSourceType
*/
LLGL_EXPORT bool IsShaderSourceCode(const ShaderSourceType type);

/**
\brief Returns true if the specified shader source type is either ShaderSourceType::BinaryBuffer or ShaderSourceType::BinaryFile.
\see ShaderSourceType
*/
LLGL_EXPORT bool IsShaderSourceBinary(const ShaderSourceType type);


} // /namespace LLGL


#endif



// ================================================================================
