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
#include "VertexAttribute.h"
#include "ShaderUniformFlags.h"
#include "ResourceFlags.h"
#include "BufferFlags.h"
#include <string>
#include <vector>


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
\remarks Specifies which shader stages are affected by a state change,
e.g. to which shader stages a constant buffer is set.
For the render systems, which do not support these flags, always all shader stages are affected.
*/
struct StageFlags
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

    inline ShaderDescriptor(const std::string& entryPoint, const std::string& target, long flags = 0) :
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
    \note Only supported with: HLSL, SPIR-V.
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

/**
\brief Shader reflection descriptor structure.
\remarks Contains all information of resources and attributes that can be queried from a shader program.
\see ShaderProgram::QueryReflectionDesc
*/
struct ShaderReflectionDescriptor
{
    /**
    \brief Shader reflection resource view structure.
    \remarks A mapping between this structure and a binding descriptor may look like this:
    \code
    auto myShaderReflectionDesc = myShaderProgram->QueryReflectionDesc();
    LLGL::PipelineLayoutDescriptor myPipelineLayoutDesc;
    for (const auto& myResourceView : myShaderReflectionDesc) {
        BindingDescriptor myBindingDesc;
        myBindingDesc.type          = myResourceView.type;
        myBindingDesc.stageFlags    = myResourceView.stageFlags;
        myBindingDesc.slot          = myResourceView.slot;
        myBindingDesc.arraySize     = myResourceView.arraySize;
        myPipelineLayoutDesc.bindings.push_back(myBindingDesc);
    }
    \endcode
    \see BindingDescriptor
    */
    struct ResourceView
    {
        //! Name of the shader resource, i.e. the identifier used in the shader.
        std::string         name;

        //! Resource view type for this layout binding. By default ResourceType::Undefined.
        ResourceType        type                = ResourceType::Undefined;

        /**
        \brief Specifies in which shader stages the resource is located. By default 0.
        \remarks This can be a bitwise OR combination of the StageFlags bitmasks.
        \see StageFlags
        */
        long                stageFlags          = 0;

        //! Specifies the zero-based binding slot. By default 0.
        std::uint32_t       slot                = 0;

        /**
        \brief Specifies the number of binding slots for an array resource. By default 1.
        \note For Vulkan, this number specifies the size of an array of resources (e.g. an array of uniform buffers).
        */
        std::uint32_t       arraySize           = 1;

        /**
        \brief Specifies the size (in bytes) for a constant buffer resource.
        \remarks Additional attribute exclusively used for constant buffer resources.
        For all other resources, i.e. when 'type' is not equal to 'ResourceType::ConstantBuffer', this attribute is zero.
        \see ResourceType::ConstantBuffer
        */
        std::uint32_t       constantBufferSize  = 0;

        /**
        \brief Specifies the sub-type of a storage buffer resource.
        \remarks Additional attribute exclusively used for storage buffer resources.
        */
        StorageBufferType   storageBufferType   = StorageBufferType::Undefined;
    };

    //! List of all vertex attributes.
    std::vector<VertexAttribute>        vertexAttributes;

    //! List of all stream-output attributes.
    std::vector<StreamOutputAttribute>  streamOutputAttributes;

    //! List of all shader reflection resource views.
    std::vector<ResourceView>           resourceViews;

    /**
    \brief List of all uniforms.
    \note Only supported with: OpenGL, Vulkan.
    */
    std::vector<UniformDescriptor>      uniforms;
};


} // /namespace LLGL


#endif



// ================================================================================
